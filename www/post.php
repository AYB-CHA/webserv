<?php
    echo "<pre>";
    print_r($_POST);
    print_r($_FILES);
    echo file_get_contents('php://input');
    // move_uploaded_file($_FILES['myfile']['tmp_name'], $_FILES['myfile']['name']);
