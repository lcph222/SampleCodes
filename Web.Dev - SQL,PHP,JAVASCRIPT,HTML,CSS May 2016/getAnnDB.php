<?php

require("/var/www/html/php/common.php"); //common codes, start session
require('/var/www/html/php/checkLogin.php'); //check if user logIn

echo json_encode(getAnnList());

function getAnnList(){

	$DBA = new DBi();

	if(!($DBA->connectToSql('Annotation'))){
			echo("failed connection to sql db");
			return false;
	}	

	$query = "select AnnotationName from annotations";

	if($DBA->executeSelectStatment($query,$result)){

		while($row =mysqli_fetch_assoc($result)){
			$AssocArray[] = $row['AnnotationName'];
    	}
		return $AssocArray;
	}	

	return array();

}

?>