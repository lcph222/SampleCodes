<?php

  
    // This if statement checks to determine whether the registration form has been submitted
    // If it has, then the registration code is run, otherwise the form is displayed
    if(!empty($_POST))
    {

        require("/var/www/html/php/common.php");

        // First we execute our common code to connection to the database and start the session
        $DBi = new DBi();

        if(!($DBi->connectToSql("Member"))){
            echo("failed connection to sql db");
            die("failed connection to database");
        }
    

        // Ensure that the user has entered a non-empty username
        if(empty($_POST['username']))
        {
            // Note that die() is generally a terrible way of handling user errors
            // like this.  It is much better to display the error with the form
            // and allow the user to correct their mistake.  However, that is an
            // exercise for you to implement yourself.
            die("Please enter a username.");
        }
        
        // Ensure that the user has entered a non-empty password
        if(empty($_POST['password']))
        {
            die("Please enter a password.");
        }
        
        // Make sure the user entered a valid E-Mail address
        // filter_var is a useful PHP function for validating form input, see:
        // http://us.php.net/manual/en/function.filter-var.php
        // http://us.php.net/manual/en/filter.filters.php
        if(!filter_var($_POST['email'], FILTER_VALIDATE_EMAIL))
        {
            die("Invalid E-Mail Address");
        }
        
        // We will use this SQL query to see whether the username entered by the
        // user is already in use.  A SELECT query is used to retrieve data from the database.
        // :username is a special token, we will substitute a real value in its place when
        // we execute the query.
        $query = " SELECT 1 FROM users WHERE username = ? ";
        
        // This contains the definitions for any special tokens that we place in
        // our SQL query.  In this case, we are defining a value for the token
        // :username.  It is possible to insert $_POST['username'] directly into
        // your $query string; however doing so is very insecure and opens your
        // code up to SQL injection exploits.  Using tokens prevents this.
        // For more information on SQL injections, see Wikipedia:
        // http://en.wikipedia.org/wiki/SQL_Injection
        $types="s";
        $params[1] = &$_POST['username'];
        
        if($DBi->checkElementInQueryExist($query,$types,$params,$result)){
 
            die("This username is already in use");
       
        }

        // Now we perform the same type of check for the email address, in order
        // to ensure that it is unique.
        $query = " SELECT 1 FROM users WHERE email = ?";
        
        $types="s";
        $params[1] = &$_POST['email'];
        
        if($DBi->checkElementInQueryExist($query,$types,$params,$result)){
 
            die("This email address is already registered");
        }
        
        // An INSERT query is used to add new rows to a database table.
        // Again, we are using special tokens (technically called parameters) to
        // protect against SQL injection attacks.
        $query = " INSERT INTO users ( username, password, salt, email) VALUES (?,?,?,?)";
        
        // A salt is randomly generated here to protect again brute force attacks
        // and rainbow table attacks.  The following statement generates a hex
        // representation of an 8 byte salt.  Representing this in hex provides
        // no additional security, but makes it easier for humans to read.
        // For more information:
        // http://en.wikipedia.org/wiki/Salt_%28cryptography%29
        // http://en.wikipedia.org/wiki/Brute-force_attack
        // http://en.wikipedia.org/wiki/Rainbow_table
        $salt = dechex(mt_rand(0, 2147483647)) . dechex(mt_rand(0, 2147483647));
        
        // This hashes the password with the salt so that it can be stored securely
        // in your database.  The output of this next statement is a 64 byte hex
        // string representing the 32 byte sha256 hash of the password.  The original
        // password cannot be recovered from the hash.  For more information:
        // http://en.wikipedia.org/wiki/Cryptographic_hash_function
        $password = hash('sha256', $_POST['password'] . $salt);
        
        // Next we hash the hash value 65536 more times.  The purpose of this is to
        // protect against brute force attacks.  Now an attacker must compute the hash 65537
        // times for each guess they make against a password, whereas if the password
        // were hashed only once the attacker would have been able to make 65537 different 
        // guesses in the same amount of time instead of only one.
        for($round = 0; $round < 65536; $round++)
        {
            $password = hash('sha256', $password . $salt);
        }
        
        // Here we prepare our tokens for insertion into the SQL query.  We do not
        // store the original password; only the hashed version of it.  We do store
        // the salt (in its plaintext form; this is not a security risk).
        $types="ssss";
        $params[1] = &$_POST['username'];
        $params[2] = &$password;
        $params[3] = &$salt;
        $params[4] = &$_POST['email'];

    
        if(!$DBi->dynamicBindSql($query,$types,$params,$result)){
            die("Can't create new user");
        }else{
            //create user folder
            $userDir = "/var/www/html/USERS/" .$params[1];
            if(!mkdir($userDir,0755))
                die("Can't create user's folder");
        }
        

        // This redirects the user back to the login page after they register
        header("Location: /link/buildDB.php");
        
        // Calling die or exit after performing a redirect using the header function
        // is critical.  The rest of your PHP script will continue to execute and
        // will be sent to the user if you do not die or exit.
        die("Redirecting to /link/buildDB.php");
    }
    
?>


<!DOCTYPE html>
<html>
<head>

<?php include '/var/www/html/include/header.php';?>

</head>

<body>

<h1 class = "container ">Register</h1>
<form class = "container" action="register.php" method="post">
    Username:<br />
    <input type="text" name="username" value="" />
    <br /><br />
    E-Mail:<br />
    <input type="text" name="email" value="" />
    <br /><br />
    Password:<br />
    <input type="password" name="password" value="" />
    <br /><br />
    <input type="submit" value="Register" />
</form>

</body>

</html>