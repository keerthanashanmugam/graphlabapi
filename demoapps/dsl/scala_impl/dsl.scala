import scala.util.parsing.input.PagedSeqReader
import scala.collection.immutable.PagedSeq
import scala.util.parsing.combinator.syntactical._

sealed trait GLType { 
  def repr:String
  override def toString = repr
}
case object StringType extends GLType { def repr = "string" }
sealed trait NumType extends GLType
case object IntType extends NumType { def repr = "int" }
case object DoubleType extends NumType { def repr = "double" }
case object BoolType extends GLType { def repr = "bool" }
case class StructType(contents:List[(String,GLType)]) extends GLType {
  def repr = throw new RuntimeException("Bug Eric to implement this")
}
case object UserFunType extends GLType { def repr = "user_funs*" }

sealed trait Stm { override def toString = repr; def repr:String }
sealed trait Exp { override def toString = repr; def repr:String }

sealed trait Lit extends Exp

case class StringLit(s:String) extends Lit { def repr = "\"" + s + "\"" }
case class IntLit(i:Int) extends Lit { def repr = i.toString }
case class DoubleLit(d:Double) extends Lit { def repr = d.toString }
case class BoolLit(b:Boolean) extends Lit { def repr = b.toString }
case class UserFun(s:String) extends Lit { def repr = "user_functions->" + s }

sealed trait BinOp { def repr:String }
case object Add extends BinOp { def repr = "+" }
case object Sub extends BinOp { def repr = "-" }
case object Mul extends BinOp { def repr = "*" }
case object Div extends BinOp { def repr = "/" }
case object Lte extends BinOp { def repr = "<=" }
case object Gte extends BinOp { def repr = ">=" }
case object Lt extends BinOp { def repr = "<" }
case object Gt extends BinOp { def repr = ">" }
case object Eq extends BinOp { def repr = "==" }
case object Neq extends BinOp { def repr = "!=" }

case class Identifier(id:String) extends Exp { def repr = id }

case class BinExp(op:BinOp,a:Exp,b:Exp) extends Exp { def repr = a + " " + op.repr + " " + b }
case class FunCall(fun:Exp,args:List[Exp]) extends Exp { def repr = fun + "(" + util.commaSep(args.map(_.toString)) + ")" }

case class IfStm(cond:Exp,tbranch:List[Stm],fbranch:List[Stm]) extends Stm {
  def repr = "if (" + cond + ") {" + "\n" + tbranch.map(_ + "\n").reduceLeft(_ + _) + "}" + (if(fbranch != List()) {" else {\n" + fbranch.map(_ + "\n").reduceLeft(_ + _) + "}"} else {""})
}
case class VarDec(vname:String,vtype:GLType,init:Exp) extends Stm { def repr = vtype + " " + vname + " = " + init + ";"}
case class Assign(vname:String,e:Exp) extends Stm { def repr = vname + " = " + e + ";"}
case class StmExp(e:Exp) extends Stm { def repr = e.repr + ";"}
case class Print(e:Exp) extends Stm { def repr = "std::cout << " + e + " << std::endl;" }

case class FunDef(u:List[(String,GLType)],fname:String,params:List[(String,GLType)],body:List[Stm]) {
  def repr = "extern \"C\" void " + fname + util.genParams(u ++ params) + "{\n" + body.map(_ + "\n").reduceLeft(_ + _) + "}"
}

object GLParser extends StandardTokenParsers {

  val keywords = List("def","true","false","if","var","else","print","vertex_program","get_vertex_data","reduce_neighbors","set_vertex_data","signal_neighbors")
  val types = List("bool","int","double","string","vertextype","edgetype")

  lexical.reserved ++= keywords
  lexical.reserved ++= types

  lexical.delimiters ++= List("{","}","(",")",".",",","+","-","*","/","<",">","=",":","!")

  def numLit:Parser[Int] = opt("-") ~ numericLit ^^ {
    case None ~ n => n.toInt
    case Some(_) ~ n => (-1)*n.toInt
  }

  def dLit:Parser[Double] = numLit ~ "." ~ numericLit ^^ {
    case n ~ _ ~ m => ((n.toString) ++ "." ++ (m.toString)).toDouble
  }

  def number:Parser[Lit] = (
    (dLit ^^ { case d => new DoubleLit(d) }) |
    (numLit ^^ { case n => new IntLit(n) }))

  def args:Parser[List[Exp]] = "(" ~> repsep(exp,",") <~ ")"

  def fnargs:Parser[(Exp,List[Exp])=>Exp] = "" ^^ {case _ => (x:Exp,y:List[Exp]) => FunCall(x,y)}

  def exp:Parser[Exp] = cmpexp
  def cmpexp:Parser[Exp] = chainl1(addexp,cmpop)
  def addexp:Parser[Exp] = chainl1(mulexp,addop)
  def mulexp:Parser[Exp] = chainl1(fnexp,mulop)
  def fnexp:Parser[Exp] = baseexp ~ opt(args) ^^ {
    case b ~ None => b
    case b ~ Some(a) => FunCall(b,a)
  }
  def baseexp:Parser[Exp] = lit | reserved | ident ^^ {case s => Identifier(s)} | "(" ~> exp <~ ")"
  def reserved:Parser[Exp] = ("get_vertex_data" | "reduce_neighbors" | "set_vertex_data" | "signal_neighbors") ^^ {case s => UserFun(s) }

  def lit:Parser[Lit] = ( number | stringLit ^^ { case s => StringLit(s) } |
    ("true" ^^ {case _ => BoolLit(true)}) |
    ("false" ^^ {case _ => BoolLit(false)})
  )

  type EComb = ((Exp,Exp)=>Exp)

  def bExp(b:BinOp):(Any=>EComb) = {(n:Any) => {
    (x:Exp,y:Exp) => BinExp(b,x,y)
  } }

  def cmpop:Parser[EComb] = (
    ("<" ~ "=" ^^ { bExp(Lte) }) |
    (">" ~ "=" ^^ { bExp(Gte) }) |
    (">" ^^ { bExp(Gt) }) |
    ("<" ^^ { bExp(Lt) }) |
    ("=" ~ "=" ^^ { bExp(Eq) }) |
    ("!" ~ "=" ^^ { bExp(Neq) })
  )

  def addop:Parser[EComb] = (
    ("+" ^^ { bExp(Add) }) |
    ("-" ^^ { bExp(Sub) })
  )

  def mulop:Parser[EComb] = (
    ("*" ^^ { bExp(Mul) }) |
    ("/" ^^ { bExp(Div) })
  )

  def stm:Parser[Stm] = (ifstm | vardec | assign | print )
  def stms:Parser[List[Stm]] = rep(stm)
  def block:Parser[List[Stm]] = "{" ~> stms <~ "}"
  def ifstm:Parser[IfStm] = "if" ~> exp ~ block ~ opt(("else" ~> block)) ^^ {
    case e ~ tb ~ Some(fb) => IfStm(e,tb,fb)
    case e ~ tb ~ None => IfStm(e,tb,List())
  }

  def vardec:Parser[VarDec] = "var" ~> ident ~ ":" ~ typ ~ "=" ~ exp ^^ {case n ~ _ ~ t ~  _ ~ e => VarDec(n,t,e)}
  def typ:Parser[GLType] = (
    ("int" ^^ {case _ => IntType})
    | ("bool" ^^ {case _ => BoolType})
    | ("string" ^^ {case _ => StringType})
    | ("double" ^^ {case _ => DoubleType})
    | ("{" ~> rep(ident ~ typ ^^ {case i ~ t => (i,t)}) <~ "}" ^^ {case l => StructType(l)})
  )

  def assign:Parser[Assign] = ident ~ "=" ~ exp ^^ {case i ~ _ ~ e => Assign(i,e)}
  def stmexp:Parser[StmExp] = exp ^^ {case e => StmExp(e)}
  def print:Parser[Print] = "print" ~> "(" ~> exp <~ ")" ^^ {case e => Print(e)}

  def fundef:Parser[FunDef] = "def" ~> opt("vertex_program") ~ ident ~ params ~ block ^^ {case None ~ i ~ p ~ b => FunDef(List(),i,p,b)
    case Some(_) ~ i ~ p ~ b => FunDef(List(("user_functions",UserFunType)),i,p,b)}


  def params:Parser[List[(String,GLType)]] = "(" ~> repsep(ident ~ ":" ~ typ ^^ {case i ~ _ ~ t => (i,t)},",") <~ ")"


  def prog:Parser[List[FunDef]] = phrase(rep(fundef))


  def runParser(fname:String) = {
    val res = prog(new lexical.Scanner(
      new PagedSeqReader(PagedSeq fromFile fname))) match {
      case Success(x,_) => x
      case e:NoSuccess => throw new RuntimeException(e.toString)
    }
    val includes = "#include <iostream>\n"
    println(includes + res.map(_.repr).reduceLeft(_ + _))

  }
}

object util {
  def genParams(l:List[(String,GLType)]) = {
    util.commaSep(l.map(x => x._2 + " " + x._1))
  }


  def commaSep(a:List[String]):String = {
    a match {
      case List() => "()"
      case x => "(" + x.map(_ + ",").reduceLeft(_ + _).dropRight(1) + ")"
    }
  }
}

object main {

  def main(args:Array[String]) = {
    GLParser.runParser(args(0))
  }
}
