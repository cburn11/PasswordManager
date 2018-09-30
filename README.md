# PasswordManager
Win32/C++ simple password manager that uses the Bcrypt library to encrypt .xml password files. 

The application itself is NOT secure. To load
the decrypted file, the application first decrypts and saves the decrypted file contents to a temporary file, then loads the contents of the
file into memory, and deletes (with DeleteFile) the temporary file. The decrypted contents, along with the key, remain in memory until the
file is closed or the application exits.

<img src="https://github.com/cburn11/PasswordManager/raw/master/test-accounts.PNG">

Common keyboard accelerators:

(ctrl + shift + L) Launches the default web browser and navigates to the URL of the selected password entry and launches the clipboard monitor
with the username and password from the entry. The clipboard monitor watches for (ctrl + v) paste operations and cycles the username/password
on the clipboard. (ctrl + q) cycles the username and password without a paste operation. (ctrl + x) detaches the clipboard monitor.

(PgUp / PgDown) when the account editor is open cycles through the account entries

Persistent password file:

The password file is an xml file:

&lt;passwordmanager&gt;
  
  &lt;accounts&gt;
  
   &lt;account&gt;</br>
      &lt;id&gt;&lt;/id&gt;</br>
      &lt;name&gt;&lt;/name&gt;</br>
      &lt;url&gt;&lt;/url&gt;</br>
      &lt;username&gt;&lt;/username&gt;</br>
      &lt;password&gt;&lt;/password&gt;</br>
      &lt;description&gt;&lt;/description&gt;</br>
      &lt;usernamefield&gt;&lt;/usernamefield&gt;</br>
      &lt;passwordfield&gt;&lt;/passwordfield&gt;</br>
   &lt;/account&gt;
  
    ...
    
  &lt;/accounts&gt;
  
&lt;/passwordmanager&gt;

The usernamefield and passwordfield tags are holdovers from when the password manager used them to fill in the username and password in IE
forms. 

.pwm files are encrypted with AES256 using the bcrypt library.

test-account-data-encrypted.pwm is test-account-data.xml encrypted with the key 123

# PSWDGEN

PSWDGEN is a simple password generator that uses the bcrypt random number generator to create password with symbols, digits, spaces, lower case letters and capital letters.

<img src="https://github.com/cburn11/PasswordManager/raw/master/pswdgen.PNG">

# checkhaveibeenpwned

checkhaveibeenpwned is a simple gui app that checks if an account is listed as "breached" by <a href="https://haveibeenpwned.com">haveibeenpwned.com</a>. The project depends on <a href="https://github.com/Microsoft/cpprestsdk">Microsoft's cpprestsdk</a> for http request and json processing. It is convenient to add to the tools menu.

<img src="https://github.com/cburn11/PasswordManager/raw/master/checkhaveibeenpwned.jpg">
