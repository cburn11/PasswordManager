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

app.Visible = True

app.SetProperty "Mode", "Settings"
app.SetProperty "Mode", "Regular"

keepSleeping=true
while keepSleeping
    WScript.Sleep 200
wend