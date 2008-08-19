package edu.stanford.atom.sti.client.comm.corba;


/**
* edu/stanford/atom/sti/client/comm/corba/TRowHelper.java .
* Generated by the IDL-to-Java compiler (portable), version "3.2"
* from ./client.idl
* Sunday, August 10, 2008 10:54:00 AM PDT
*/

abstract public class TRowHelper
{
  private static String  _id = "IDL:STI_Client_Server/TRow/TRow:1.0";

  public static void insert (org.omg.CORBA.Any a, edu.stanford.atom.sti.client.comm.corba.TRow that)
  {
    org.omg.CORBA.portable.OutputStream out = a.create_output_stream ();
    a.type (type ());
    write (out, that);
    a.read_value (out.create_input_stream (), type ());
  }

  public static edu.stanford.atom.sti.client.comm.corba.TRow extract (org.omg.CORBA.Any a)
  {
    return read (a.create_input_stream ());
  }

  private static org.omg.CORBA.TypeCode __typeCode = null;
  private static boolean __active = false;
  synchronized public static org.omg.CORBA.TypeCode type ()
  {
    if (__typeCode == null)
    {
      synchronized (org.omg.CORBA.TypeCode.class)
      {
        if (__typeCode == null)
        {
          if (__active)
          {
            return org.omg.CORBA.ORB.init().create_recursive_tc ( _id );
          }
          __active = true;
          org.omg.CORBA.StructMember[] _members0 = new org.omg.CORBA.StructMember [2];
          org.omg.CORBA.TypeCode _tcOf_members0 = null;
          _tcOf_members0 = org.omg.CORBA.ORB.init ().create_string_tc (0);
          _tcOf_members0 = org.omg.CORBA.ORB.init ().create_sequence_tc (0, _tcOf_members0);
          _tcOf_members0 = org.omg.CORBA.ORB.init ().create_alias_tc (edu.stanford.atom.sti.client.comm.corba.TStringSeqHelper.id (), "TStringSeq", _tcOf_members0);
          _members0[0] = new org.omg.CORBA.StructMember (
            "val",
            _tcOf_members0,
            null);
          _tcOf_members0 = org.omg.CORBA.ORB.init ().get_primitive_tc (org.omg.CORBA.TCKind.tk_boolean);
          _members0[1] = new org.omg.CORBA.StructMember (
            "done",
            _tcOf_members0,
            null);
          __typeCode = org.omg.CORBA.ORB.init ().create_struct_tc (edu.stanford.atom.sti.client.comm.corba.TRowHelper.id (), "TRow", _members0);
          __active = false;
        }
      }
    }
    return __typeCode;
  }

  public static String id ()
  {
    return _id;
  }

  public static edu.stanford.atom.sti.client.comm.corba.TRow read (org.omg.CORBA.portable.InputStream istream)
  {
    edu.stanford.atom.sti.client.comm.corba.TRow value = new edu.stanford.atom.sti.client.comm.corba.TRow ();
    value.val = edu.stanford.atom.sti.client.comm.corba.TStringSeqHelper.read (istream);
    value.done = istream.read_boolean ();
    return value;
  }

  public static void write (org.omg.CORBA.portable.OutputStream ostream, edu.stanford.atom.sti.client.comm.corba.TRow value)
  {
    edu.stanford.atom.sti.client.comm.corba.TStringSeqHelper.write (ostream, value.val);
    ostream.write_boolean (value.done);
  }

}
