package edu.stanford.atom.sti.client.comm.corba;

/**
* edu/stanford/atom/sti/client/comm/corba/TStatusLevelHolder.java .
* Generated by the IDL-to-Java compiler (portable), version "3.2"
* from ./client.idl
* Sunday, August 10, 2008 10:54:00 AM PDT
*/

public final class TStatusLevelHolder implements org.omg.CORBA.portable.Streamable
{
  public edu.stanford.atom.sti.client.comm.corba.TStatusLevel value = null;

  public TStatusLevelHolder ()
  {
  }

  public TStatusLevelHolder (edu.stanford.atom.sti.client.comm.corba.TStatusLevel initialValue)
  {
    value = initialValue;
  }

  public void _read (org.omg.CORBA.portable.InputStream i)
  {
    value = edu.stanford.atom.sti.client.comm.corba.TStatusLevelHelper.read (i);
  }

  public void _write (org.omg.CORBA.portable.OutputStream o)
  {
    edu.stanford.atom.sti.client.comm.corba.TStatusLevelHelper.write (o, value);
  }

  public org.omg.CORBA.TypeCode _type ()
  {
    return edu.stanford.atom.sti.client.comm.corba.TStatusLevelHelper.type ();
  }

}
