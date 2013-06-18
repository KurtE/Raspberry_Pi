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
        Me.LCDGroup = New System.Windows.Forms.GroupBox()
        Me.LCDLB = New System.Windows.Forms.ListBox()
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
        Me.RobotIPAddrGP.SuspendLayout()
        Me.ComGB.SuspendLayout()
        Me.LCDGroup.SuspendLayout()
        Me.PMTerminal.SuspendLayout()
        Me.XBeeDLContextMenu.SuspendLayout()
        Me.ContextMenuStrip1.SuspendLayout()
        Me.PortGP.SuspendLayout()
        Me.SuspendLayout()
        '
        'RobotIPAddrGP
        '
        Me.RobotIPAddrGP.Controls.Add(Me.RobotIPLB)
        Me.RobotIPAddrGP.Location = New System.Drawing.Point(153, 10)
        Me.RobotIPAddrGP.Name = "RobotIPAddrGP"
        Me.RobotIPAddrGP.Size = New System.Drawing.Size(164, 47)
        Me.RobotIPAddrGP.TabIndex = 54
        Me.RobotIPAddrGP.TabStop = False
        Me.RobotIPAddrGP.Text = "Robot IP Address"
        '
        'RobotIPLB
        '
        Me.RobotIPLB.FormattingEnabled = True
        Me.RobotIPLB.Location = New System.Drawing.Point(1, 19)
        Me.RobotIPLB.Name = "RobotIPLB"
        Me.RobotIPLB.Size = New System.Drawing.Size(152, 21)
        Me.RobotIPLB.TabIndex = 1
        '
        'Connect
        '
        Me.Connect.Anchor = CType((System.Windows.Forms.AnchorStyles.Top Or System.Windows.Forms.AnchorStyles.Right), System.Windows.Forms.AnchorStyles)
        Me.Connect.Location = New System.Drawing.Point(442, 27)
        Me.Connect.Name = "Connect"
        Me.Connect.Size = New System.Drawing.Size(97, 23)
        Me.Connect.TabIndex = 52
        Me.Connect.Text = "Connect"
        Me.Connect.UseVisualStyleBackColor = True
        '
        'ComGB
        '
        Me.ComGB.Controls.Add(Me.ComLB)
        Me.ComGB.Location = New System.Drawing.Point(11, 10)
        Me.ComGB.Name = "ComGB"
        Me.ComGB.Size = New System.Drawing.Size(136, 47)
        Me.ComGB.TabIndex = 51
        Me.ComGB.TabStop = False
        Me.ComGB.Text = "Commander Comm Port"
        '
        'ComLB
        '
        Me.ComLB.FormattingEnabled = True
        Me.ComLB.Location = New System.Drawing.Point(1, 19)
        Me.ComLB.Name = "ComLB"
        Me.ComLB.Size = New System.Drawing.Size(123, 21)
        Me.ComLB.TabIndex = 1
        '
        'LCDGroup
        '
        Me.LCDGroup.Anchor = CType((((System.Windows.Forms.AnchorStyles.Top Or System.Windows.Forms.AnchorStyles.Bottom) _
            Or System.Windows.Forms.AnchorStyles.Left) _
            Or System.Windows.Forms.AnchorStyles.Right), System.Windows.Forms.AnchorStyles)
        Me.LCDGroup.Controls.Add(Me.LCDLB)
        Me.LCDGroup.Location = New System.Drawing.Point(14, 62)
        Me.LCDGroup.Name = "LCDGroup"
        Me.LCDGroup.Size = New System.Drawing.Size(528, 187)
        Me.LCDGroup.TabIndex = 53
        Me.LCDGroup.TabStop = False
        Me.LCDGroup.Text = "LCD"
        '
        'LCDLB
        '
        Me.LCDLB.Dock = System.Windows.Forms.DockStyle.Fill
        Me.LCDLB.FormattingEnabled = True
        Me.LCDLB.Location = New System.Drawing.Point(3, 16)
        Me.LCDLB.Name = "LCDLB"
        Me.LCDLB.SelectionMode = System.Windows.Forms.SelectionMode.MultiSimple
        Me.LCDLB.Size = New System.Drawing.Size(522, 168)
        Me.LCDLB.TabIndex = 0
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
        Me.PortGP.Location = New System.Drawing.Point(323, 10)
        Me.PortGP.Name = "PortGP"
        Me.PortGP.Size = New System.Drawing.Size(113, 47)
        Me.PortGP.TabIndex = 58
        Me.PortGP.TabStop = False
        Me.PortGP.Text = "Port"
        '
        'Port
        '
        Me.Port.Location = New System.Drawing.Point(0, 15)
        Me.Port.Name = "Port"
        Me.Port.Size = New System.Drawing.Size(100, 20)
        Me.Port.TabIndex = 0
        '
        'Form1
        '
        Me.AutoScaleDimensions = New System.Drawing.SizeF(6.0!, 13.0!)
        Me.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font
        Me.ClientSize = New System.Drawing.Size(554, 259)
        Me.Controls.Add(Me.PortGP)
        Me.Controls.Add(Me.RobotIPAddrGP)
        Me.Controls.Add(Me.Connect)
        Me.Controls.Add(Me.ComGB)
        Me.Controls.Add(Me.LCDGroup)
        Me.MinimumSize = New System.Drawing.Size(570, 170)
        Me.Name = "Form1"
        Me.Text = "Wifi Commander"
        Me.RobotIPAddrGP.ResumeLayout(False)
        Me.ComGB.ResumeLayout(False)
        Me.LCDGroup.ResumeLayout(False)
        Me.PMTerminal.ResumeLayout(False)
        Me.XBeeDLContextMenu.ResumeLayout(False)
        Me.ContextMenuStrip1.ResumeLayout(False)
        Me.PortGP.ResumeLayout(False)
        Me.PortGP.PerformLayout()
        Me.ResumeLayout(False)

    End Sub
    Friend WithEvents RobotIPAddrGP As System.Windows.Forms.GroupBox
    Friend WithEvents RobotIPLB As System.Windows.Forms.ComboBox
    Friend WithEvents Connect As System.Windows.Forms.Button
    Friend WithEvents ComGB As System.Windows.Forms.GroupBox
    Friend WithEvents ComLB As System.Windows.Forms.ComboBox
    Friend WithEvents LCDGroup As System.Windows.Forms.GroupBox
    Friend WithEvents LCDLB As System.Windows.Forms.ListBox
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

End Class
