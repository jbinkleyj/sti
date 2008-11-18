/*
 * TabbedEditor.java
 *
 * Created on November 17, 2008, 10:05 AM
 */

package edu.stanford.atom.sti.client.gui.FileEditorTab;

import javax.swing.*;
import java.util.Vector;
import java.io.*;
import javax.swing.text.*;

/**
 *
 * @author  Owner
 */
public class TabbedEditor extends javax.swing.JPanel {
        
    private enum fileError {ReadOnly, ReadError, FileIsOpen, NoError}
        
    private Vector<TabbedDocument> tabbedDocumentVector = new Vector<TabbedDocument>();
    
    /** Creates new form TabbedEditor */
    public TabbedEditor() {
        initComponents();
    }
        
    public void tabIsModified(int tabIndex)
            throws ArrayIndexOutOfBoundsException {

        if (tabIndex >= tabbedDocumentVector.size() || tabIndex < 0) {
            // Creates exception if the position of the caret is outside of the
            // size of the document?
            throw new ArrayIndexOutOfBoundsException(tabIndex);
        }

        tabbedDocumentVector.elementAt(tabIndex).setModifed(true);
    }

    public void tabIsNotModified(int tabIndex)
            throws ArrayIndexOutOfBoundsException {

        if (tabIndex >= tabbedDocumentVector.size() || tabIndex < 0) {
            throw new ArrayIndexOutOfBoundsException(tabIndex);
        }

        tabbedDocumentVector.elementAt(tabIndex).setModifed(false);
    }
    
    public void addEditorTab(TabbedDocument tabbedDocument) {
        textEditorTabbedPane.addTab(tabbedDocument.getTabTitle(),
                tabbedDocument);
    }

    public void removeEditorTab() {
        removeEditorTab(textEditorTabbedPane.getSelectedIndex());
    }

    public void removeEditorTab(int tabIndex)
            throws ArrayIndexOutOfBoundsException {

        if (tabIndex >= tabbedDocumentVector.size() || tabIndex < 0) {
            throw new ArrayIndexOutOfBoundsException(tabIndex);
        }
        textEditorTabbedPane.removeTabAt(tabIndex);
        tabbedDocumentVector.remove(tabIndex);
        //Reindex so TabbedDocument indicies match JTabbedPane indicies
        for (int i = tabIndex; i < tabbedDocumentVector.size(); i++) {
            tabbedDocumentVector.elementAt(i).setTabIndex(i);
        }
    }

    /**
     * Creates a blank new document in a new tab with title Untitled#, where # 
     * is a number starting from 1 that increments each time a new file is made. 
     * The File associated with the newly created TabbedDocument is null.
     */
   
    public void createNewFile() {

        tabbedDocumentVector.addElement(
                new TabbedDocument(null, textEditorTabbedPane,
                tabbedDocumentVector.size()));

        addEditorTab(tabbedDocumentVector.lastElement());

        // Ensure the new tab is active
        textEditorTabbedPane.setSelectedIndex(
                tabbedDocumentVector.lastElement().getTabIndex());

        tabbedDocumentVector.lastElement().getTextPane().setText("");

        tabIsNotModified(tabbedDocumentVector.lastElement().getTabIndex());
    }
    
    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        toolbarPanel = new javax.swing.JPanel();
        newButton = new javax.swing.JButton();
        openButton = new javax.swing.JButton();
        saveButton = new javax.swing.JButton();
        jSeparator1 = new javax.swing.JSeparator();
        jSplitPane2 = new javax.swing.JSplitPane();
        jLabel1 = new javax.swing.JLabel();
        jComboBox1 = new javax.swing.JComboBox();
        parseButton = new javax.swing.JButton();
        textEditorSplitPane = new javax.swing.JSplitPane();
        textEditorTabbedPane = new javax.swing.JTabbedPane();
        parserScrollPane = new javax.swing.JScrollPane();
        parserTextArea = new javax.swing.JTextArea();

        setPreferredSize(new java.awt.Dimension(600, 600));

        newButton.setText("New");
        newButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                newButtonActionPerformed(evt);
            }
        });

        openButton.setText("Open");

        saveButton.setText("Save");

        jSeparator1.setOrientation(javax.swing.SwingConstants.VERTICAL);

        jSplitPane2.setBorder(null);
        jSplitPane2.setDividerSize(2);
        jSplitPane2.setOrientation(javax.swing.JSplitPane.VERTICAL_SPLIT);

        jLabel1.setText("Main File:");
        jSplitPane2.setTopComponent(jLabel1);

        jComboBox1.setModel(new javax.swing.DefaultComboBoxModel(new String[] { "Item 1", "Item 2", "Item 3", "Item 4" }));
        jSplitPane2.setBottomComponent(jComboBox1);

        parseButton.setText("Parse");

        javax.swing.GroupLayout toolbarPanelLayout = new javax.swing.GroupLayout(toolbarPanel);
        toolbarPanel.setLayout(toolbarPanelLayout);
        toolbarPanelLayout.setHorizontalGroup(
            toolbarPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(toolbarPanelLayout.createSequentialGroup()
                .addGap(6, 6, 6)
                .addComponent(newButton, javax.swing.GroupLayout.PREFERRED_SIZE, 53, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(openButton, javax.swing.GroupLayout.PREFERRED_SIZE, 59, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(saveButton)
                .addGap(14, 14, 14)
                .addComponent(jSeparator1, javax.swing.GroupLayout.PREFERRED_SIZE, 11, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(parseButton)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addComponent(jSplitPane2, javax.swing.GroupLayout.PREFERRED_SIZE, 219, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap(94, Short.MAX_VALUE))
        );
        toolbarPanelLayout.setVerticalGroup(
            toolbarPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(toolbarPanelLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(toolbarPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(toolbarPanelLayout.createSequentialGroup()
                        .addGroup(toolbarPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                            .addComponent(newButton, javax.swing.GroupLayout.PREFERRED_SIZE, 34, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addComponent(openButton, javax.swing.GroupLayout.PREFERRED_SIZE, 34, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addComponent(saveButton, javax.swing.GroupLayout.PREFERRED_SIZE, 34, javax.swing.GroupLayout.PREFERRED_SIZE))
                        .addContainerGap())
                    .addGroup(toolbarPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING, false)
                        .addComponent(parseButton, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                        .addComponent(jSplitPane2, javax.swing.GroupLayout.DEFAULT_SIZE, 36, Short.MAX_VALUE))))
            .addComponent(jSeparator1, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.DEFAULT_SIZE, 56, Short.MAX_VALUE)
        );

        textEditorSplitPane.setDividerLocation(475);
        textEditorSplitPane.setDividerSize(4);
        textEditorSplitPane.setOrientation(javax.swing.JSplitPane.VERTICAL_SPLIT);
        textEditorSplitPane.setResizeWeight(1.0);

        textEditorTabbedPane.setTabPlacement(javax.swing.JTabbedPane.BOTTOM);
        textEditorTabbedPane.setMinimumSize(new java.awt.Dimension(38, 1));
        textEditorSplitPane.setTopComponent(textEditorTabbedPane);

        parserScrollPane.setMinimumSize(new java.awt.Dimension(23, 50));

        parserTextArea.setColumns(20);
        parserTextArea.setRows(5);
        parserScrollPane.setViewportView(parserTextArea);

        textEditorSplitPane.setRightComponent(parserScrollPane);

        javax.swing.GroupLayout layout = new javax.swing.GroupLayout(this);
        this.setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addComponent(toolbarPanel, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
            .addComponent(textEditorSplitPane, javax.swing.GroupLayout.DEFAULT_SIZE, 600, Short.MAX_VALUE)
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
                .addComponent(toolbarPanel, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(textEditorSplitPane, javax.swing.GroupLayout.DEFAULT_SIZE, 538, Short.MAX_VALUE))
        );
    }// </editor-fold>//GEN-END:initComponents

    private void newButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_newButtonActionPerformed
        createNewFile();
    }//GEN-LAST:event_newButtonActionPerformed
    
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JComboBox jComboBox1;
    private javax.swing.JLabel jLabel1;
    private javax.swing.JSeparator jSeparator1;
    private javax.swing.JSplitPane jSplitPane2;
    private javax.swing.JButton newButton;
    private javax.swing.JButton openButton;
    private javax.swing.JButton parseButton;
    private javax.swing.JScrollPane parserScrollPane;
    private javax.swing.JTextArea parserTextArea;
    private javax.swing.JButton saveButton;
    private javax.swing.JSplitPane textEditorSplitPane;
    private javax.swing.JTabbedPane textEditorTabbedPane;
    private javax.swing.JPanel toolbarPanel;
    // End of variables declaration//GEN-END:variables
    
}
