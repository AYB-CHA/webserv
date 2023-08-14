<?php session_start(); ?>

<!DOCTYPE html>
    <html lang="en">
    <head>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <title>Document</title>
    </head>
    <body>
    <?php

        if (!isset($_SESSION['login_status']))
            $_SESSION['login_status'] = false;

        if ($_SERVER['REQUEST_METHOD'] == 'POST')
        {
            if ($_POST['username'] == 'admin' && $_POST['password'] == 'admin')
                $_SESSION['login_status'] = true;
        }

    if (isset($_SESSION['login_status']) && $_SESSION['login_status'] == true)
        echo "<h1>YOU ARE LOGGED IN</h1>";
    else
    {
        echo <<<EOF
            <form method="POST" action="/login.php">
                <input placeholder="username" name="username">
                <br />
                <br />
                <input placeholder="password" name="password">
                <br />
                <br />
                <br />
                <button>LOGIN</button>
            </form>
        EOF;
    }
    ?>
    </body>
</html>