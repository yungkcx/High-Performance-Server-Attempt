<!DOCTYPE HTML5>
<html>
	<head>
		<title>This is title</title>
		<link rel="stylesheet" type="text/css" href="style.css" />
		<meta charset="utf-8" />
		<script src="action.js" ></script>
	</head>
	<body id = "ossqa">
		<form action="" method="post">
			<input id="input" type="input" name="abc" />
            <input type="submit" name="Submit" />
		</form>
	</body>
</html>

<?php
echo $_POST['abc'];
?>
