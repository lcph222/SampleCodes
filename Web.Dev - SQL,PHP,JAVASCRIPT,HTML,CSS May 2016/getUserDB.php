<?php

require("/var/www/html/php/common.php"); //common codes, start session
require('/var/www/html/php/checkLogin.php'); //check if user logIn

if(!empty(($_SESSION['user']['username']))){
	$list = getDBList($_SESSION['user']['username']);
	//echo var_dump($list);

	echo json_encode($list);
}

//got here means user is log in
function getDBList($userName){

	global $DBM;

	$query = "select Annotation from usersDatabases where username=?";

	$params = array();
	$params[1] = &$userName;
	$types = "s";

	//dynamic way, return true if there's result
	if($DBM->checkElementInQueryExist($query,$types,$params,$result)){
		while($row =mysqli_fetch_assoc($result)){
			$AssocArray[] = $row['Annotation'];
    	}
		return $AssocArray;
	}

	return array();
}

?>