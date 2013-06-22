<Global.Microsoft.VisualBasic.CompilerServices.DesignerGenerated()> _
Partial Class Form1
    Inherits System.Windows.Forms.Form

    'Form overrides dispose to clean up the component list.
    <System.Diagnostics.DebuggerNonUserCode()> _
    Protected Overrides Sub Dispose(ByVal disposing As Boolean)
        Try
            If disposing AndAlso components IsNot Nothing Then
                components.Dispose()
            End If
        Finally
            MyBase.Dispose(disposing)
        End Try
    End Sub

    'Required by the Windows Form Designer
    Private components As System.ComponentModel.IContainer

    'NOTE: The following procedure is required by the Windows Form Designer
    'It can be modified using the Windows Form Designer.  
    'Do not modify it using the code editor.
    <System.Diagnostics.DebuggerStepThrough()> _
    Private Sub InitializeComponent()
        Me.components = New System.ComponentModel.Container()
        Me.RobotIPAddrGP = New System.Windows.Forms.GroupBox()
        Me.RobotIPLB = New System.Windows.Forms.ComboBox()
        Me.Connect = New System.Windows.Forms.Button()
        Me.ComGB = New System.Windows.Forms.GroupBox()
        Me.ComLB = New System.Windows.Forms.ComboBox()
        Me.Timer1 = New System.Windows.Forms.Timer(Me.components)
        Me.CommThread = New System.ComponentModel.BackgroundWorker()
        Me.PMTerminal = New System.Windows.Forms.ContextMenuStrip(Me.components)
        Me.LBTerminalCount = New System.Windows.Forms.ToolStripMenuItem()
        Me.LBTerminalSelectAll = New System.Windows.Forms.ToolStripMenuItem()
        Me.LBTerminalCopytoClipBoard = New System.Windows.Forms.ToolStripMenuItem()
        Me.LBTerminalClear = New System.Windows.Forms.ToolStripMenuItem()
        Me.XBeeDLContextMenu = New System.Windows.Forms.ContextMenuStrip(Me.components)
        Me.XBDLCM_DeleteItem = New System.Windows.Forms.ToolStripMenuItem()
        Me.XBDLCM_Clear = New System.Windows.Forms.ToolStripMenuItem()
        Me.ContextMenuStrip1 = New System.Windows.Forms.ContextMenuStrip(Me.components)
        Me.ToolStripMenuItem1 = New System.Windows.Forms.ToolStripMenuItem()
        Me.ToolStripMenuItem2 = New System.Windows.Forms.ToolStripMenuItem()
        Me.PortGP = New System.Windows.Forms.GroupBox()
        Me.Port = New System.Windows.Forms.TextBox()
        Me.HorizontalSplit = New System.Windows.Forms.SplitContainer()
        Me.WebBrowser1 = New System.Windows.Forms.WebBrowser()
        Me.LCDLB = New System.Windows.Forms.ListBox()
        Me.WebPageGB = New System.Windows.Forms.GroupBox()
        Me.WebPage = New System.Windows.Forms.TextBox()
        Me.RobotIPAddrGP.SuspendLayout()
        Me.ComGB.SuspendLayout()
        Me.PMTerminal.SuspendLayout()
        Me.XBeeDLContextMenu.SuspendLayout()
        Me.ContextMenuStrip1.SuspendLayout()
        Me.PortGP.SuspendLayout()
        CType(Me.HorizontalSplit, System.ComponentModel.ISupportInitialize).BeginInit()
        Me.HorizontalSplit.Panel1.SuspendLayout()
        Me.HorizontalSplit.Panel2.SuspendLayout()
        Me.HorizontalSplit.SuspendLayout()
        Me.WebPageGB.SuspendLayout()
        Me.SuspendLayout()
        '
        'RobotIPAddrGP
        '
        Me.RobotIPAddrGP.Controls.Add(Me.RobotIPLB)
        Me.RobotIPAddrGP.Location = New System.Drawing.Point(117, 12)
        Me.RobotIPAddrGP.Name = "RobotIPAddrGP"
        Me.RobotIPAddrGP.Size = New System.Drawing.Size(138, 47)
        Me.RobotIPAddrGP.TabIndex = 54
        Me.RobotIPAddrGP.TabStop = False
        Me.RobotIPAddrGP.Text = "Robot IP Address"
        '
        'RobotIPLB
        '
        Me.RobotIPLB.FormattingEnabled = True
        Me.RobotIPLB.Location = New System.Drawing.Point(6, 19)
        Me.RobotIPLB.Name = "RobotIPLB"
        Me.RobotIPLB.Size = New System.Drawing.Size(152, 21)
        Me.RobotIPLB.TabIndex = 1
        '
        'Connect
        '
        Me.Connect.Anchor = CType((System.Windows.Forms.AnchorStyles.Top Or System.Windows.Forms.AnchorStyles.Right), System.Windows.Forms.AnchorStyles)
        Me.Connect.Location = New System.Drawing.Point(597, 27)
        Me.Connect.Name = "Connect"
        Me.Connect.Size = New System.Drawing.Size(68, 23)
        Me.Connect.TabIndex = 52
        Me.Connect.Text = "Connect"
        Me.Connect.UseVisualStyleBackColor = True
        '
        'ComGB
        '
        Me.ComGB.Controls.Add(Me.ComLB)
        Me.ComGB.Location = New System.Drawing.Point(11, 10)
        Me.ComGB.Name = "ComGB"
        Me.ComGB.Size = New System.Drawing.Size(100, 47)
        Me.ComGB.TabIndex = 51
        Me.ComGB.TabStop = False
        Me.ComGB.Text = "Commander Port"
        '
        'ComLB
        '
        Me.ComLB.FormattingEnabled = True
        Me.ComLB.Location = New System.Drawing.Point(5, 19)
        Me.ComLB.Name = "ComLB"
        Me.ComLB.Size = New System.Drawing.Size(119, 21)
        Me.ComLB.TabIndex = 1
        '
        'Timer1
        '
        Me.Timer1.Interval = 250
        '
        'CommThread
        '
        Me.CommThread.WorkerReportsProgress = True
        Me.CommThread.WorkerSupportsCancellation = True
        '
        'PMTerminal
        '
        Me.PMTerminal.Items.AddRange(New System.Windows.Forms.ToolStripItem() {Me.LBTerminalCount, Me.LBTerminalSelectAll, Me.LBTerminalCopytoClipBoard, Me.LBTerminalClear})
        Me.PMTerminal.Name = "PMTerminal"
        Me.PMTerminal.Size = New System.Drawing.Size(123, 92)
        '
        'LBTerminalCount
        '
        Me.LBTerminalCount.Name = "LBTerminalCount"
        Me.LBTerminalCount.Size = New System.Drawing.Size(122, 22)
        Me.LBTerminalCount.Text = "Count..."
        '
        'LBTerminalSelectAll
        '
        Me.LBTerminalSelectAll.Name = "LBTerminalSelectAll"
        Me.LBTerminalSelectAll.Size = New System.Drawing.Size(122, 22)
        Me.LBTerminalSelectAll.Text = "Select All"
        '
        'LBTerminalCopytoClipBoard
        '
        Me.LBTerminalCopytoClipBoard.Name = "LBTerminalCopytoClipBoard"
        Me.LBTerminalCopytoClipBoard.Size = New System.Drawing.Size(122, 22)
        Me.LBTerminalCopytoClipBoard.Text = "Copy"
        '
        'LBTerminalClear
        '
        Me.LBTerminalClear.Name = "LBTerminalClear"
        Me.LBTerminalClear.Size = New System.Drawing.Size(122, 22)
        Me.LBTerminalClear.Text = "Clear"
        '
        'XBeeDLContextMenu
        '
        Me.XBeeDLContextMenu.Items.AddRange(New System.Windows.Forms.ToolStripItem() {Me.XBDLCM_DeleteItem, Me.XBDLCM_Clear})
        Me.XBeeDLContextMenu.Name = "PMTerminal"
        Me.XBeeDLContextMenu.Size = New System.Drawing.Size(135, 48)
        '
        'XBDLCM_DeleteItem
        '
        Me.XBDLCM_DeleteItem.Name = "XBDLCM_DeleteItem"
        Me.XBDLCM_DeleteItem.Size = New System.Drawing.Size(134, 22)
        Me.XBDLCM_DeleteItem.Text = "Delete Item"
        '
        'XBDLCM_Clear
        '
        Me.XBDLCM_Clear.Name = "XBDLCM_Clear"
        Me.XBDLCM_Clear.Size = New System.Drawing.Size(134, 22)
        Me.XBDLCM_Clear.Text = "Clear List"
        '
        'ContextMenuStrip1
        '
        Me.ContextMenuStrip1.Items.AddRange(New System.Windows.Forms.ToolStripItem() {Me.ToolStripMenuItem1, Me.ToolStripMenuItem2})
        Me.ContextMenuStrip1.Name = "PMTerminal"
        Me.ContextMenuStrip1.Size = New System.Drawing.Size(135, 48)
        '
        'ToolStripMenuItem1
        '
        Me.ToolStripMenuItem1.Name = "ToolStripMenuItem1"
        Me.ToolStripMenuItem1.Size = New System.Drawing.Size(134, 22)
        Me.ToolStripMenuItem1.Text = "Delete Item"
        '
        'ToolStripMenuItem2
        '
        Me.ToolStripMenuItem2.Name = "ToolStripMenuItem2"
        Me.ToolStripMenuItem2.Size = New System.Drawing.Size(134, 22)
        Me.ToolStripMenuItem2.Text = "Clear List"
        '
        'PortGP
        '
        Me.PortGP.Controls.Add(Me.Port)
        Me.PortGP.Location = New System.Drawing.Point(261, 12)
        Me.PortGP.Name = "PortGP"
        Me.PortGP.Size = New System.Drawing.Size(97, 47)
        Me.PortGP.TabIndex = 58
        Me.PortGP.TabStop = False
        Me.PortGP.Text = "Port"
        '
        'Port
        '
        Me.Port.Location = New System.Drawing.Point(6, 19)
        Me.Port.Name = "Port"
        Me.Port.Size = New System.Drawing.Size(85, 20)
        Me.Port.TabIndex = 0
        '
        'HorizontalSplit
        '
        Me.HorizontalSplit.Anchor = CType((((System.Windows.Forms.AnchorStyles.Top Or System.Windows.Forms.AnchorStyles.Bottom) _
            Or System.Windows.Forms.AnchorStyles.Left) _
            Or System.Windows.Forms.AnchorStyles.Right), System.Windows.Forms.AnchorStyles)
        Me.HorizontalSplit.Location = New System.Drawing.Point(11, 63)
        Me.HorizontalSplit.Name = "HorizontalSplit"
        Me.HorizontalSplit.Orientation = System.Windows.Forms.Orientation.Horizontal
        '
        'HorizontalSplit.Panel1
        '
        Me.HorizontalSplit.Panel1.Controls.Add(Me.WebBrowser1)
        '
        'HorizontalSplit.Panel2
        '
        Me.HorizontalSplit.Panel2.Controls.Add(Me.LCDLB)
        Me.HorizontalSplit.Size = New System.Drawing.Size(664, 586)
        Me.HorizontalSplit.SplitterDistance = 500
        Me.HorizontalSplit.TabIndex = 60
        '
        'WebBrowser1
        '
        Me.WebBrowser1.Anchor = CType((((System.Windows.Forms.AnchorStyles.Top Or System.Windows.Forms.AnchorStyles.Bottom) _
            Or System.Windows.Forms.AnchorStyles.Left) _
            Or System.Windows.Forms.AnchorStyles.Right), System.Windows.Forms.AnchorStyles)
        Me.WebBrowser1.Location = New System.Drawing.Point(5, 15)
        Me.WebBrowser1.MinimumSize = New System.Drawing.Size(20, 20)
        Me.WebBrowser1.Name = "WebBrowser1"
        Me.WebBrowser1.Size = New System.Drawing.Size(649, 479)
        Me.WebBrowser1.TabIndex = 60
        Me.WebBrowser1.Url = New System.Uri("http://192.168.2.100:8080", System.UriKind.Absolute)
        '
        'LCDLB
        '
        Me.LCDLB.Anchor = CType((((System.Windows.Forms.AnchorStyles.Top Or System.Windows.Forms.AnchorStyles.Bottom) _
            Or System.Windows.Forms.AnchorStyles.Left) _
            Or System.Windows.Forms.AnchorStyles.Right), System.Windows.Forms.AnchorStyles)
        Me.LCDLB.FormattingEnabled = True
        Me.LCDLB.Location = New System.Drawing.Point(6, 12)
        Me.LCDLB.Name = "LCDLB"
        Me.LCDLB.SelectionMode = System.Windows.Forms.SelectionMode.MultiSimple
        Me.LCDLB.Size = New System.Drawing.Size(655, 56)
        Me.LCDLB.TabIndex = 1
        '
        'WebPageGB
        '
        Me.WebPageGB.Controls.Add(Me.WebPage)
        Me.WebPageGB.Location = New System.Drawing.Point(364, 12)
        Me.WebPageGB.Name = "WebPageGB"
        Me.WebPageGB.Size = New System.Drawing.Size(227, 47)
        Me.WebPageGB.TabIndex = 59
        Me.WebPageGB.TabStop = False
        Me.WebPageGB.Text = "Web Page"
        '
        'WebPage
        '
        Me.WebPage.Location = New System.Drawing.Point(6, 19)
        Me.WebPage.Name = "WebPage"
        Me.WebPage.Size = New System.Drawing.Size(232, 20)
        Me.WebPage.TabIndex = 0
        '
        'Form1
        '
        Me.AutoScaleDimensions = New System.Drawing.SizeF(6.0!, 13.0!)
        Me.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font
        Me.ClientSize = New System.Drawing.Size(690, 661)
        Me.Controls.Add(Me.WebPageGB)
        Me.Controls.Add(Me.HorizontalSplit)
        Me.Controls.Add(Me.PortGP)
        Me.Controls.Add(Me.RobotIPAddrGP)
        Me.Controls.Add(Me.Connect)
        Me.Controls.Add(Me.ComGB)
        Me.MinimumSize = New System.Drawing.Size(700, 300)
        Me.Name = "Form1"
        Me.Text = "Wifi Commander"
        Me.RobotIPAddrGP.ResumeLayout(False)
        Me.ComGB.ResumeLayout(False)
        Me.PMTerminal.ResumeLayout(False)
        Me.XBeeDLContextMenu.ResumeLayout(False)
        Me.ContextMenuStrip1.ResumeLayout(False)
        Me.PortGP.ResumeLayout(False)
        Me.PortGP.PerformLayout()
        Me.HorizontalSplit.Panel1.ResumeLayout(False)
        Me.HorizontalSplit.Panel2.ResumeLayout(False)
        CType(Me.HorizontalSplit, System.ComponentModel.ISupportInitialize).EndInit()
        Me.HorizontalSplit.ResumeLayout(False)
        Me.WebPageGB.ResumeLayout(False)
        Me.WebPageGB.PerformLayout()
        Me.ResumeLayout(False)

    End Sub
    Friend WithEvents RobotIPAddrGP As System.Windows.Forms.GroupBox
    Friend WithEvents RobotIPLB As System.Windows.Forms.ComboBox
    Friend WithEvents Connect As System.Windows.Forms.Button
    Friend WithEvents ComGB As System.Windows.Forms.GroupBox
    Friend WithEvents ComLB As System.Windows.Forms.ComboBox
    Friend WithEvents Timer1 As System.Windows.Forms.Timer
    Friend WithEvents CommThread As System.ComponentModel.BackgroundWorker
    Friend WithEvents PMTerminal As System.Windows.Forms.ContextMenuStrip
    Friend WithEvents LBTerminalCount As System.Windows.Forms.ToolStripMenuItem
    Friend WithEvents LBTerminalSelectAll As System.Windows.Forms.ToolStripMenuItem
    Friend WithEvents LBTerminalCopytoClipBoard As System.Windows.Forms.ToolStripMenuItem
    Friend WithEvents LBTerminalClear As System.Windows.Forms.ToolStripMenuItem
    Friend WithEvents XBeeDLContextMenu As System.Windows.Forms.ContextMenuStrip
    Friend WithEvents XBDLCM_DeleteItem As System.Windows.Forms.ToolStripMenuItem
    Friend WithEvents XBDLCM_Clear As System.Windows.Forms.ToolStripMenuItem
    Friend WithEvents ContextMenuStrip1 As System.Windows.Forms.ContextMenuStrip
    Friend WithEvents ToolStripMenuItem1 As System.Windows.Forms.ToolStripMenuItem
    Friend WithEvents ToolStripMenuItem2 As System.Windows.Forms.ToolStripMenuItem
    Friend WithEvents PortGP As System.Windows.Forms.GroupBox
    Friend WithEvents Port As System.Windows.Forms.TextBox
    Friend WithEvents HorizontalSplit As System.Windows.Forms.SplitContainer
    Friend WithEvents WebBrowser1 As System.Windows.Forms.WebBrowser
    Friend WithEvents LCDLB As System.Windows.Forms.ListBox
    Friend WithEvents WebPageGB As System.Windows.Forms.GroupBox
    Friend WithEvents WebPage As System.Windows.Forms.TextBox

End Class
