<?php
    for ($i=0; $i < 10; $i++) { 
        usleep(1000000);
        echo "abc" . "<br>";
    }
    echo "Hello, World!";
?>
<form action="post.php" method="POST" enctype="multipart/form-data">
    <input type="text" name="name">
    <input type="file" name="myfile">
    <br>
    <button>SEND</button>
</form>