<?php
error_reporting(-1);
ini_set('display_errors',1);
header('Content-Type: text/html; charset=utf-8');
$page = (isset($_GET['page']) ? $_GET['page'] : 'main');
?>
<html>
<head>
<title>Index.php</title>
</head>
<body>
<header>
    <nav>
        <a href="index.php?page=main">Main page</a> |
        <a href="index.php?page=contacts">Contacts</a>
    </nav>
</header>

<?php include basename($page).'.php'; ?>

<footer>
    footer of the site
</footer>
</body>
</html>