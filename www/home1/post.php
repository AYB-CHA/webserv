<?php
    echo "<pre>";
    print_r($_POST);
    print_r($_FILES);
    move_uploaded_file($_FILES['myfile']['tmp_name'], $_FILES['myfile']['name']);
