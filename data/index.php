<?php
// Generate a random number between 1 and 100
$randomNumber = rand(1, 100);

// Initialize variables
$guess = null;
$attempts = 0;
$message = null;

// Check if a guess has been submitted
if (isset($_POST['guess'])) {
    $guess = $_POST['guess'];
    $attempts++;
    
    // Validate the guess
    if (!is_numeric($guess)) {
        $message = "Please enter a valid number.";
    } else {
        // Check if the guess is correct
        if ($guess == $randomNumber) {
            $message = "Congratulations! You guessed the correct number in $attempts attempts.";
            $randomNumber = rand(1, 100); // Generate a new random number for the next round
            $attempts = 0; // Reset the attempts
        } elseif ($guess < $randomNumber) {
            $message = "Try a higher number.";
        } else {
            $message = "Try a lower number.";
        }
    }
}
?>

<!DOCTYPE html>
<html>
<head>
    <title>Number Guessing Game</title>
</head>
<body>
    <h1>Number Guessing Game</h1>
    <p>Guess a number between 1 and 100:</p>
    <form method="POST">
        <input type="text" name="guess" placeholder="Enter your guess" />
        <input type="submit" value="Guess" />
    </form>
    <p><?php echo $message; ?></p>
</body>
</html>
