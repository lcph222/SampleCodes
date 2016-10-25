<?php

//This class can be used to connect to MySQL server, and execute prepared/non-prepared statement
//can also be used to convert sql results to array

class DBi{

	public $conn;
	
	//call this function to connect to the server, need basic info and serverName and database name
	function connectToSql($ServerName, $DBName, $UserName,$Password){

		// Create connection
		$this->conn = new mysqli($ServerName, $UserName, $Password);

		// Check connection
		if ($this->conn->connect_error) {
    			#die("Connection failed: " . $this->conn->connect_error);

    			return false;
		}

		//using database
		if(!mysqli_select_db($this->conn,$DBName)){

			$this->closeConn();
			return false;
		}

	   return true;
	}

	function closeConn(){

		mysqli_close($this->conn);
	}

	 //return false if error in db or false executed
	//this function will execute prepared statement that is stored in $query and return true if executed successfully and return result in $result 
	function dynamicBindSql($query,$types,$params,&$result){

		if($stmt = ($this->conn)->prepare($query)){ //prepare pass

			$types_params = array();
			$types_params[] = &$types; //copy type to first element

			if(is_array($params)){ //check if params is an array
				$numParams = count($params);
				for($i = 1; $i <= $numParams; $i++){ //for each param
					$types_params[] = &$params[$i]; 
				}
			}else {
				$types_params[] = &$params;
			}
		

			/* use call_user_func_array, as $stmt->bind_param('s', $param); does not accept params array */
			call_user_func_array(array($stmt, 'bind_param'), $types_params);

			if($stmt->execute()){
				//echo "executed successfully\n";
				/* Fetch result to array */
				$result = $stmt->get_result();

 			}else{

 				return false;
 			}

		}else{

	    	return false;
		}

		return true;
	}

	//this function will execute "Select" prepared statement, return true if there's a result
	function checkElementInQueryExist($query,$types,$params,&$result){

		$result = null;

		if($this->dynamicBindSql($query,$types,$params,$result)){
			if($result->num_rows > 0)
				return true;

			return false;
		}

		return -1;
	}

	//parse sql result to array
	function parseSqlTableToArray($result, &$Array){

		$Array = array();

		while($row =mysqli_fetch_assoc($result)){
			$Array[] = $row;
    	}

	}

	//execute select statement query stored in $sql (not prepared statement)
	function executeSelectStatment($sql,&$result){

		if($result = $this->conn->query($sql)) {
    		if($result->num_rows > 0)
				return true;
			return false;
		} else {

    		return false;
		}
	}

	//execute insertion query (not prepared statement), return true if successful
	function executeInsertStatment($sql,&$result){

		if($result = $this->conn->query($sql)) {
    		return true;
		} else {
			echo $this->conn->error."\n";
    		return false;
		}
	}

}//class database

?>