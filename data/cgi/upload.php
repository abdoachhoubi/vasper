<?php
if ($_SERVER["REQUEST_METHOD"] == "POST") {
    $uploadDir = "uploads/"; // Specify the directory where you want to save the uploaded files
    $allowedExtensions = ["jpg", "jpeg", "png", "gif"]; // Allowed file extensions

    if (!file_exists($uploadDir)) {
        mkdir($uploadDir, 0777, true);
    }

    if (isset($_FILES["fileToUpload"])) {
        $file = $_FILES["fileToUpload"];
        $fileName = $file["name"];
        $fileTmpName = $file["tmp_name"];
        $fileType = $file["type"];
        $fileSize = $file["size"];
        $fileError = $file["error"];
		// Priniting the file details in stderror
		fwrite(STDERR, "File Name: " . $fileName . "\n");

        $fileExtension = pathinfo($fileName, PATHINFO_EXTENSION);

        if (in_array(strtolower($fileExtension), $allowedExtensions)) {
            if ($fileError === 0) {
                $newFileName = uniqid() . "." . $fileExtension;
                $destination = $uploadDir . $newFileName;

                if (move_uploaded_file($fileTmpName, $destination)) {
                    echo "File uploaded successfully!";
                } else {
                    echo "Error uploading the file.";
                }
            } else {
                echo "There was an error uploading your file.";
            }
        } else {
            echo "Invalid file type. Only JPG, JPEG, PNG, and GIF files are allowed.";
        }
    }
}
?>

<!DOCTYPE html>
<html>
<head>
    <title>File Upload</title>
</head>
<body>
    <form action="upload.php" method="post" enctype="multipart/form-data">
        <input type="file" name="fileToUpload" id="fileToUpload">
        <input type="submit" value="Upload File" name="submit">
    </form>
</body>
</html>
