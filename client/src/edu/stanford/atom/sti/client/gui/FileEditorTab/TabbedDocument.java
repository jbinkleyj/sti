/*
 * TabbedDocument.java
 *
 * Created on November 17, 2008, 4:06 PM
 */

package edu.stanford.atom.sti.client.gui.FileEditorTab;

import edu.stanford.atom.sti.RemoteFileServer.NetworkFileChooser.*;
import javax.swing.JScrollPane;
import javax.swing.*;
import java.io.*;
import javax.swing.text.*;

/**
 *
 * @author  Owner
 */
public class TabbedDocument extends JScrollPane {
    
    public static int untitledDocCount = 0;
    
    private boolean isLocal = true;
    private File localFile = null;
    
    private NetworkFileSystem nfs = null;
    private String path = null;
    
    private String tabFileName = null;
    private int tabIndex;
    private boolean modified = false;
    
    private AbstractDocument abstractDocumentLocal;
    private JTabbedPane parentTabbedPane;

    
    public TabbedDocument(String Path, NetworkFileSystem networkFileSystem, JTabbedPane tabbedPane, int TabIndex) {
        this(tabbedPane, TabIndex);
        saveDocument(Path, networkFileSystem);

    }
    public TabbedDocument(File file, JTabbedPane tabbedPane, int TabIndex) {
        this(tabbedPane, TabIndex);
        saveDocument(file);
    }
    private TabbedDocument(JTabbedPane tabbedPane, int TabIndex) {
        parentTabbedPane = tabbedPane;
        tabIndex = TabIndex;
        initComponents();
        addDocumentListener();
    }
    public void saveDocument(String Path, NetworkFileSystem networkFileSystem) {
        path = Path;
        nfs = networkFileSystem;
        isLocal = false;
        setTabTitle();
    }    
    public void saveDocument(File file) {
        localFile = file;
        isLocal = true;
        setTabTitle();
    }
    private void addDocumentListener() {
        StyledDocument styledDocTemp = mainTextPane.getStyledDocument();
        
        if (styledDocTemp instanceof AbstractDocument) {
            abstractDocumentLocal = (AbstractDocument)styledDocTemp;
            abstractDocumentLocal.addDocumentListener(
                    new TabbedDocumentListener(this));
        } else {
            System.err.println("Text pane's document isn't an AbstractDocument.");
        }        
    }
    public void setModifed(boolean Modified) {
        modified = Modified;
        if(modified) {
            parentTabbedPane.setTitleAt(tabIndex, getTabTitle() + " *");
        }
        else {
            parentTabbedPane.setTitleAt(tabIndex, getTabTitle());
        }
    }
    
    public boolean isModifed() {
        return modified;
    }
    
    public boolean isLocalFile() {
        return isLocal;
    }
    
    public boolean isNetworkFile() {
        return !isLocalFile();
    }
    
    public boolean isNewFile() {
        return (localFile == null && nfs == null);
    }
    
    public boolean canWrite() {
        if(isLocal) {
            if(localFile != null)
                return localFile.canWrite();
            else
                return true;
        }
        else {      //network file
            return ( !nfs.isReadOnly(path) );
        }
    }
    
    public void setTabTitle() {

        if(isLocal) {
            if(localFile != null)
                tabFileName = localFile.getName();
        }
        else {
            if(nfs != null && path != null)
                tabFileName = nfs.shortFileName(path);
        }
        
        if(isNewFile()) {
            untitledDocCount++;
            tabFileName = "Untitled"
                    + Integer.toString(untitledDocCount);
        }
    }

    public String getTabTitle() {
        // Place <> around tab name to distiguish read only
        if(!canWrite())
            return ("<" + tabFileName + ">");
        else if(isLocalFile())
            return (tabFileName + "(local)");
        else
            return tabFileName;
    }

    public String getFileName() {
        return tabFileName;
    }

    public int getTabIndex() {
        return tabIndex;
    }
    public void setTabIndex(int index) {
        tabIndex = index;
    }

    public String getServerAddress() {
        if(isLocalFile())
            return null;
        else
            return nfs.getFullAddress();
    }
    
    public File getFile() {
        return localFile;
    }
    public String getPath() {
        return path;
    }
    @Override
    public String toString() {
        return getTabTitle();
    }
    public NetworkFileSystem getNetworkFileSystem() {
        return nfs;
    }
    public javax.swing.JTextPane getTextPane() {
        return mainTextPane;
    }
    
    public boolean equals(File file) {
        if(file != null && localFile != null)
            return localFile.equals(file);
        return false;
    }
    public boolean equals(NetworkFileSystem networkFileSystem, String file) {
        if(nfs != null && path != null && networkFileSystem != null && file != null) {
            System.out.println("Network equals: " + nfs.equals(networkFileSystem) + ", " + path.equals(file));
            return (nfs.equals(networkFileSystem) && path.equals(file));
        }
        return false;
    }
    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        mainTextPane = new javax.swing.JTextPane();

        setMinimumSize(new java.awt.Dimension(1, 23));
        setViewportView(mainTextPane);
    }// </editor-fold>//GEN-END:initComponents
    
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JTextPane mainTextPane;
    // End of variables declaration//GEN-END:variables
    
}
