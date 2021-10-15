dim app

set app = Wscript.CreateObject("PasswordGenerator.Application")

app.Visible = True

app.SetProperty "c_lower_case", "12"

dim password
password = app.GeneratePassword()

MsgBox  password

app.Quit()