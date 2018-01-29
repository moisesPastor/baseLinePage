awk '{
    if ($1 == "<Word") { 
         while ( $1 != "</Word>") {
		getline;
         } 
    } else
      print
}' $1
