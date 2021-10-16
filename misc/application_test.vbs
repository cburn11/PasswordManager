Function IApplicationEvents_OnQuit()

	MsgBox "OnQuit"
	keepSleeping = false

End Function

Function IApplicationEvents_PropertyChange(name, value)

	dim str
	str = name & " = " & value
	MsgBox str
	

End Function

Function IApplicationEvents_PasswordGenerated(password)

	MsgBox password

End Function


dim app
dim keepSleeping

set app = Wscript.CreateObject("PasswordGenerator.Application", "IApplicationEvents_")
'set app = Wscript.CreateObject("PasswordGenerator.Application")


app.Visible = True

app.SetProperty "c_lower_case", "12"

'app.Quit()



keepSleeping=true
while keepSleeping
    WScript.Sleep 200
wend