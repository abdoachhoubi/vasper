#!/usr/bin/php-cgi
<?php
// Start a session
session_start();

// Check if a user name is already stored in the session
if (isset($_SESSION['username'])) {
    $username = $_SESSION['username'];
} else {
    $username = '';
}

// Check if a "username" cookie is set
if (isset($_COOKIE['username'])) {
    $cookieUsername = $_COOKIE['username'];
} else {
    $cookieUsername = '';
}

// Handle form submission
if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    $inputUsername = isset($_POST['username']) ? $_POST['username'] : '';

    // Store the username in a session variable
    $_SESSION['username'] = $inputUsername;

    // Set a "username" cookie to remember the username
    setcookie('username', $inputUsername, time() + 3600, '/'); // Cookie expires in 1 hour

    header('Location: ' . $_SERVER['PHP_SELF']);
    exit;
}
?>

<!DOCTYPE html>
<html>
<head>
    <title>Cookie and Session Test</title>
</head>
<body>
    <h1>Cookie and Session Test</h1>
    <p>Welcome, <?php echo $username; ?></p>

    <form method="post">
        <label for="username">Enter your name:</label>
        <input type="text" name="username" id="username" value="<?php echo $username; ?>">
        <input type="submit" value="Submit">
    </form>

    <p>Username from session: <?php echo $username; ?></p>
    <p>Username from cookie: <?php echo $cookieUsername; ?></p>
</body>
</html>
