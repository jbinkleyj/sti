package edu.stanford.atom.sti.client.comm.corba;


/**
* edu/stanford/atom/sti/client/comm/corba/ParserHelper.java .
* Generated by the IDL-to-Java compiler (portable), version "3.2"
* from ./client.idl
* Sunday, August 10, 2008 10:54:00 AM PDT
*/

abstract public class ParserHelper
{
  private static String  _id = "IDL:STI_Client_Server/Parser:1.0";

  public static void insert (org.omg.CORBA.Any a, edu.stanford.atom.sti.client.comm.corba.Parser that)
  {
    org.omg.CORBA.portable.OutputStream out = a.create_output_stream ();
    a.type (type ());
    write (out, that);
    a.read_value (out.create_input_stream (), type ());
  }

  public static edu.stanford.atom.sti.client.comm.corba.Parser extract (org.omg.CORBA.Any a)
  {
    return read (a.create_input_stream ());
  }

  private static org.omg.CORBA.TypeCode __typeCode = null;
  synchronized public static org.omg.CORBA.TypeCode type ()
  {
    if (__typeCode == null)
    {
      __typeCode = org.omg.CORBA.ORB.init ().create_interface_tc (edu.stanford.atom.sti.client.comm.corba.ParserHelper.id (), "Parser");
    }
    return __typeCode;
  }

  public static String id ()
  {
    return _id;
  }

  public static edu.stanford.atom.sti.client.comm.corba.Parser read (org.omg.CORBA.portable.InputStream istream)
  {
    return narrow (istream.read_Object (_ParserStub.class));
  }

  public static void write (org.omg.CORBA.portable.OutputStream ostream, edu.stanford.atom.sti.client.comm.corba.Parser value)
  {
    ostream.write_Object ((org.omg.CORBA.Object) value);
  }

  public static edu.stanford.atom.sti.client.comm.corba.Parser narrow (org.omg.CORBA.Object obj)
  {
    if (obj == null)
      return null;
    else if (obj instanceof edu.stanford.atom.sti.client.comm.corba.Parser)
      return (edu.stanford.atom.sti.client.comm.corba.Parser)obj;
    else if (!obj._is_a (id ()))
      throw new org.omg.CORBA.BAD_PARAM ();
    else
    {
      org.omg.CORBA.portable.Delegate delegate = ((org.omg.CORBA.portable.ObjectImpl)obj)._get_delegate ();
      edu.stanford.atom.sti.client.comm.corba._ParserStub stub = new edu.stanford.atom.sti.client.comm.corba._ParserStub ();
      stub._set_delegate(delegate);
      return stub;
    }
  }

  public static edu.stanford.atom.sti.client.comm.corba.Parser unchecked_narrow (org.omg.CORBA.Object obj)
  {
    if (obj == null)
      return null;
    else if (obj instanceof edu.stanford.atom.sti.client.comm.corba.Parser)
      return (edu.stanford.atom.sti.client.comm.corba.Parser)obj;
    else
    {
      org.omg.CORBA.portable.Delegate delegate = ((org.omg.CORBA.portable.ObjectImpl)obj)._get_delegate ();
      edu.stanford.atom.sti.client.comm.corba._ParserStub stub = new edu.stanford.atom.sti.client.comm.corba._ParserStub ();
      stub._set_delegate(delegate);
      return stub;
    }
  }

}
