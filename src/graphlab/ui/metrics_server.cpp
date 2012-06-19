#include <unistd.h>
#include <string>
#include <map>
#include <utility>
#include <boost/function.hpp>

#include <graphlab/util/stl_util.hpp>
#include <graphlab/rpc/distributed_event_log.hpp>
#include <graphlab/rpc/get_last_dc_procid.hpp>

#include <graphlab/ui/mongoose/mongoose.h>
#include <graphlab/ui/metrics_server.hpp>

#include <graphlab/macros_def.hpp>


namespace graphlab {


static mg_context* metric_context = NULL;
static std::map<std::string, http_redirect_callback_type> callbacks;




static void* process_request(enum mg_event event,
                             struct mg_connection* conn,
                             const struct mg_request_info* info) {
  if (event == MG_NEW_REQUEST) {

    // get the URL being requested
    std::string url = info->uri;
    // strip the starting /
    if (url.length() >= 1) url = url.substr(1, url.length() - 1);
    // get all the variables
    std::map<std::string, std::string> variable_map;
    if (info->query_string != NULL) {
      std::string qs = info->query_string;
      std::vector<std::string> terms = strsplit(qs, "&", true);
      // now for each term..
      foreach(std::string& term, terms) {
        // get the variable name
        std::vector<std::string> key_val = strsplit(term, "=", true);
        if (key_val.size() > 0) {
          // use mg_get_var to read the actual variable.
          // since mg_get_var does http escape sequence decoding
          std::string key = key_val[0];
          char val_target[1024];
          int ret = mg_get_var(qs.c_str(), qs.length(), 
                               key.c_str(), val_target, 1024);
          if (ret >= 0) variable_map[key] = val_target;
        }
      }
    }

    // now redirect to the callback handlers. if we find one
    std::map<std::string, http_redirect_callback_type>::iterator iter = 
                                                       callbacks.find(url);

    if (iter != callbacks.end()) {
      std::pair<std::string, std::string> returnval = iter->second(variable_map);
      std::string ctype = returnval.first;
      std::string body = returnval.second;
      mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: %s\r\n"
              "Content-Length: %d\r\n" 
              "\r\n"
              "%s",
              ctype.c_str(),
              (int)body.length(), body.c_str());
    }
    else {
      std::map<std::string, http_redirect_callback_type>::iterator iter404 =
                                                          callbacks.find("404");
      std::pair<std::string, std::string> returnval;
      if (iter404 != callbacks.end()) returnval = iter404->second(variable_map);
      
      std::string ctype = returnval.first;
      std::string body = returnval.second;

      mg_printf(conn,
              "HTTP/1.1 404 Not Found\r\n"
              "Content-Type: %s\r\n"
              "Content-Length: %d\r\n" 
              "\r\n"
              "%s",
              ctype.c_str(),
              (int)body.length(), body.c_str());
    }

    return (void*)"";
  }
  else {
    return NULL;
  }
}


/*
   Simple 404 handler. Just reuturns a string "Page Not Found"
   */
std::pair<std::string, std::string> 
four_oh_four(std::map<std::string, std::string>& varmap) {
  return std::make_pair(std::string("text/html"), 
                        std::string("Page Not Found"));
}


/*
  Echo handler. Returns a html with get keys and values
 */
std::pair<std::string, std::string> 
echo(std::map<std::string, std::string>& varmap) {
  std::stringstream ret;
  std::map<std::string, std::string>::iterator iter = varmap.begin();
  ret << "<html>\n";
  while (iter != varmap.end()) {
    ret << iter->first << " = " << iter->second << "<br>\n"; 
    ++iter;
  }
  ret << "</html>\n";
  ret.flush();
  return std::make_pair(std::string("text/html"), ret.str());
}



static void fill_builtin_callbacks() {
  callbacks["404"] = four_oh_four;
  callbacks["echo"] = echo;
}


void add_metric_server_callback(std::string page, 
                                http_redirect_callback_type callback) {
  callbacks[page] = callback;
}

void launch_metric_server() {
  if (dc_impl::get_last_dc_procid() == 0) {
    const char *options[] = {"listening_ports", "8090", NULL};
    metric_context = mg_start(process_request, (void*)(&callbacks), options);
    if(metric_context == NULL) {
      logstream(LOG_ERROR) << "Unable to launch metrics server on port 8090. "
                           << "Metrics server will not be available" << std::endl;
      return;
    }
    fill_builtin_callbacks();

    char hostname[1024];
    std::string strhostname;
    if (gethostname(hostname, 1024) == 0) strhostname = hostname;
    logstream(LOG_EMPH) << "Metrics server now listening on " 
                   << "http://" << strhostname << ":8090" << std::endl;
  }
}

void stop_metric_server() {
  if (dc_impl::get_last_dc_procid() == 0 && metric_context != NULL) {
    std::cout << "Metrics server stopping." << std::endl;
    mg_stop(metric_context);
  }
}

void stop_metric_server_on_eof() {
  if (dc_impl::get_last_dc_procid() == 0 && metric_context != NULL) {
    char buff[128];
    // wait for ctrl-d
    std::cout << "Hit Ctrl-D to stop the metrics server" << std::endl;
    while (fgets(buff, 128, stdin) != NULL );
    stop_metric_server();  
  }
}

} // namespace graphlab
