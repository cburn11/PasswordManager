dim app

set app = Wscript.CreateObject("PasswordGenerator.Application")

app.Visible = True

app.SetProperty "c_lower_case", "12"

dim clower
clower = app.GetProperty("c_lower_case")

MsgBox  clower

app.Quit()