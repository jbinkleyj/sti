/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package edu.stanford.atom.sti.client.comm.bl;

import edu.stanford.atom.sti.client.comm.corba.*;
import java.util.Vector;

/**
 *
 * @author Owner
 */
public class DataManager {

    private TEvent[] events = null;
    private TChannel[] channels = null;
    private String[] files = null;
    private TOverwritten[] overwritten = null;
    private TVariable[] variables = null;

    private Parser parserRef = null;
    
    private Vector<DataManagerListener> listeners = new Vector<DataManagerListener>();
        
    public DataManager() {
        
    }
        
    public synchronized void addDataListener(DataManagerListener listener) {
        listeners.add(listener);
    }
    public synchronized void removeDataListener(DataManagerListener listener) {
        listeners.remove(listener);
    }
    
    private synchronized void fireNewParsedDataEvent() {
        DataManagerEvent event = new DataManagerEvent(this);
        
        for(int i = 0; i < listeners.size(); i++) {
            listeners.elementAt(i).getData( event );
        }
    }
    
    public void setParser(Parser parser) {
        parserRef = parser;
    }
    
    public void getParsedData() {

        if(parserRef == null || parserRef._non_existent()) {
            return;
        }

        boolean success = false;
        
        try {
            events = parserRef.events();
            channels = parserRef.channels();
            files = parserRef.files();
            overwritten = parserRef.overwritten();
            variables = parserRef.variables();
            
            success = true;

        } catch (Exception e) {
            events = null;
            channels = null;
            files = null;
            overwritten = null;
            variables = null;
 
            e.printStackTrace(System.err);
        }
        if(success) {
            fireNewParsedDataEvent();
        }
    }
    public Vector< Vector<Object> > getVariablesTableData() {
        //{"Name", "Value", "Type", "File", "Line"}
        Vector< Vector<Object> > variablesData = null;
        VariablesTableRow rowData = new VariablesTableRow();

        
        if(variables != null && files != null) {
            variablesData = new Vector< Vector<Object> >(variables.length);
            
            int fileNumber = -1;
            String fileName = "";
            TVarMixedDecode varDecode = null;
            
            for(int i = 0; i < variables.length; i++) {
                rowData.clear();
                
                fileNumber = -1;
                fileName = "";
                varDecode = null;
                
                rowData.setName(variables[i].name);
                varDecode = TVarMixedDecode.
                        createTVarMixedDecode(variables[i].value, channels);

                rowData.setValue(varDecode.getValue());
                rowData.setType(varDecode.getType());
                
                // the parser-assigned file number
                fileNumber = variables[i].pos.file;

                if (fileNumber < files.length && fileNumber >= 0) {
                    fileName = files[fileNumber];
                }

                rowData.setFile(fileName);
                rowData.setLine(variables[i].pos.line);

                variablesData.addElement(rowData.getRow());
            }
        }
        return variablesData;
    }
    
    public Vector< Vector<Object> > getOverwrittenTableData() {
        Vector< Vector<Object> > overwrittenData = null;
        OverwrittenTableRow rowData = new OverwrittenTableRow();
        
        if(overwritten != null) {
            overwrittenData = new Vector< Vector<Object> >(overwritten.length);
            
            for(int i = 0; i < overwritten.length; i++) {
                rowData.clear();
                
                rowData.setName(overwritten[i].name);
                rowData.setValue(overwritten[i].value);
                
                overwrittenData.addElement(rowData.getRow());
            }
        }
        return overwrittenData;
    }
    
        
    public Vector< Vector<Object> > getEventTableData() {
        
        Vector< Vector<Object> > eventData = null;
        EventTableRow rowData = new EventTableRow();
        
        if(events != null && channels != null && files != null) {
            
            eventData = new Vector< Vector<Object> >(events.length);
            
            int fileNumber = -1;
            int channelNumber = -1;
            TChannel tempChannel = null;
            String fileName = "";
            TChannelDecode channelDecode = null;
            TValMixedDecode valueDecode = null;

            for (int i = 0; i < events.length; i++) {
                
                fileNumber = -1;
                channelNumber = -1;
                tempChannel = null;
                fileName = "";
                channelDecode = null;
                valueDecode = null;

                rowData.clear();

                //Time
                rowData.setTime(events[i].time);
                //Value
                valueDecode = new TValMixedDecode(events[i].value);
                rowData.setValue(valueDecode.getValue());

                // the parser-assigned channel number
                channelNumber = events[i].channel;

                if (channelNumber < channels.length && channelNumber >= 0) {
                    tempChannel = channels[channelNumber];
                }

                if (tempChannel != null) {
                    //Device
                    rowData.setDevice(tempChannel.device.deviceName);
                    //Address
                    rowData.setAddress(tempChannel.device.address);
                    //Module
                    rowData.setModule(tempChannel.device.moduleNum);
                    //Channel
                    rowData.setChannel(tempChannel.channel);

                    channelDecode = new TChannelDecode(tempChannel);

                    //IO Type
                    rowData.setIO(channelDecode.IOType());
                    //Format Type
                    rowData.setType(channelDecode.ChannelType());

                }

                // the parser-assigned file number
                fileNumber = events[i].pos.file;

                if (fileNumber < files.length && fileNumber >= 0) {
                    fileName = files[fileNumber];
                }
                // File Name
                rowData.setFile(fileName);
                // File Line
                rowData.setLine(events[i].pos.line);
                
                // Add row
                eventData.addElement(rowData.getRow());
            }
        }
        return eventData;
    }
    
    private class VariablesTableRow {
        
        Object[] rowData = new Object[10];
        
        public void clear() {
            rowData = new Object[5];
        }
        public Vector<Object> getRow() {
            Vector<Object> row = new Vector<Object>(10);
            for(int i=0; i< rowData.length; i++) {
                row.addElement(rowData[i]);
            }
            return row;
        }
        public void setName(String name) {
            rowData[0] = name;
        }
        public void setValue(String value) {
            rowData[1] = value;
        }
        public void setType(String type) {
            rowData[2] = type;
        }
        public void setFile(String file) {
            rowData[3] = file;
        }
        public void setLine(int line) {
            rowData[4] = line;
        }
    }
    
    private class OverwrittenTableRow {
        String[] rowData = new String[2];
        
        public void clear() {
            rowData = new String[2];
        }
        public Vector<Object> getRow() {
            Vector<Object> row = new Vector<Object>(10);
            for(int i=0; i< rowData.length; i++) {
                row.addElement(rowData[i]);
            }
            return row;
        }
        public void setName(String name) {
            rowData[0] = name;
        }
        public void setValue(String value) {
            rowData[1] = value;
        }
    }
    
    private class EventTableRow {
        
        //   {"Time", "Value", "Device", "Address","Module",
        //    "Channel", "I/O", "Type", "File", "Line"}
        
        Object[] rowData = new Object[10];
        
        public EventTableRow() {
        }
        public void clear() {
            rowData = new Object[10];
        }
        public Vector<Object> getRow() {
            Vector<Object> row = new Vector<Object>(10);
            for(int i=0; i< rowData.length; i++) {
                row.addElement(rowData[i]);
            }
            return row;
        }
        public void setTime(double time) {
            rowData[0] = time;
        }
        public void setValue(String value) {
            rowData[1] = value;
        }
        public void setDevice(String device) {
            rowData[2] = device;
        }
        public void setAddress(String address) {
            rowData[3] = address;
        }
        public void setModule(short module) {
            rowData[4] = module;
        }
        public void setChannel(short channel) {
            rowData[5] = channel;
        }
        public void setIO(String io) {
            rowData[6] = io;
        }
        public void setType(String type) {
            rowData[7] = type;
        }
        public void setFile(String file) {
            rowData[8] = file;
        }
        public void setLine(int line) {
            rowData[9] = line;
        }
    }
    
    
}
