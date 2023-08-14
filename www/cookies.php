<?php if ($_SERVER['REQUEST_METHOD'] == "POST"){ setcookie('save', $_POST['save']); $_COOKIE["save"] = $_POST['save'];}?>

<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Document</title>
</head>
<body>
    Your saved value is: <?= $_COOKIE["save"] ?? 'nothing'; ?>
    <br>
    <br>
    <form action="./cookies.php" method="POST">
        <label>Enter a value to save:</label>
        <br>
        <input type="text" name="save">
        <br>
        <br>
        <button>SAVE</button>
    </form>
</body>
</html>