/* 
   The purpose of this program is to generate DH keys and use it to generate a common secret key among two users. 
   The secret key is then used as an AES key from encryption and decryption. 
   The file exchanges among two users are done through email, usb, etc.  
   This code uses java build in security library to generate DH key and secret key.
   It also uses the Cipher build in class with AES, ECB and PKCS5PADDING to encrypt and decrypt the text file.  
*/

import java.io.*;
import java.math.BigInteger;
import java.nio.charset.Charset;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.security.*;
import java.security.spec.*;
import java.security.interfaces.*;
import javax.crypto.*;
import javax.crypto.spec.*;
import javax.crypto.interfaces.*;


class KeyExchange {

	public static void main (String[] args) {
   	 
    	if (args.length == 0) {
        		printUsage();	// if input is empty, pring usage
    	} 
        else {	// if input is -gen generate new dh key

        	if (args[0].equals("--gen") && args.length == 3) { // check size of agrs
            	// generate a new keypair
            	KeyPair kp;
            	
                try {
                	kp = generateKeyPair(Integer.parseInt(args[1]));
            	} catch (NumberFormatException ignore) {	//catch exception 
                	try {	//generate dh key 
                    		DHPublicKey key = (DHPublicKey)KeyFactory.getInstance("DH")
                        	.generatePublic(new X509EncodedKeySpec(readByteFile(args[1])));
                    		kp = generateKeyPair(key.getParams());
                	} catch(NoSuchAlgorithmException|IOException|InvalidKeySpecException e) {
                    	e.printStackTrace();
                    	return;
                	}
            	}

            	try { //save keypair to filename args[2]
                	saveKeyPair(kp, args[2]);
            	} catch (IOException e) {
                	e.printStackTrace();
            	}

        	} else if (args[0].equals("--gen-aes") && args.length == 4) {
            	// generates the AES key using Alice's Private and Bob's Public or vice //versa
            	SecretKey key;
            	PrivateKey privateKey;
            	PublicKey publicKey;
                try { // get public and private key, if public key belong to alice, then private key //belong to bob, vice versa

                	KeyFactory kf = KeyFactory.getInstance("DH");
                	privateKey = kf.generatePrivate(new PKCS8EncodedKeySpec(readByteFile(args[1])));
                	publicKey = kf.generatePublic(new X509EncodedKeySpec(readByteFile(args[2])));
                	key = generateSecretKey(privateKey, publicKey); // generate secret key
                	writeFile(args[3], key.getEncoded());	// write to file
            	} catch (Exception e) {
                	e.printStackTrace();
                	return;
            	}
           	 
        	} else if (args[0].equals("--encrypt") && args.length == 4) { // encrypt file using aes //key in args
            	
                SecretKey key;
            	
                try {
                	key = new SecretKeySpec(readByteFile(args[1]), "AES"); //get secret AES key
                	byte[] encrypted = encrypt(readByteFile(args[2]), key);	// encrypt the text
                	writeFile(args[3], encrypted);	//save it to filename args[3]
            	}catch(IOException e) {
                	e.printStackTrace();
                	return;
            	}

        	} else if (args[0].equals("--decrypt") && args.length == 4) { // decrypt the poem using aes key
            	SecretKey key;
            	try{
                	key = new SecretKeySpec(readByteFile(args[1]), "AES"); // get aes key
                	byte[] decrypted = decrypt(readByteFile(args[2]), key);	// decrypt it
                	writeFile(args[3], decrypted);	//write it to a file
            	} catch(IOException e) {
                	e.printStackTrace();
                	return;
            	}
        	} else {
            	printUsage();
        	}
    	}
	} //end of main

	/**
 	* printUsage
 	*
 	* Prints the usage for the application
 	*/
	public static void printUsage () {
    	System.out.println("Usage: java -jar <jarfile> [--gen <SIZE|PUBLIC> <DEST>|" +
                       	"--gen-aes <PRIVATE> <PUBLIC> <DEST>|" +
                       	"--encrypt <KEY> <SRC> <DEST>|" +
                       	"--decrypt <KEY> <SRC> <DEST>]");
	}
    
	/**
 	* generateKeyPair
 	*
 	* Generates a NEW keypair for DH. Don't use this unless no keypair
 	* exists.
 	*/
	public static KeyPair generateKeyPair (Integer size) {
    	// Genererate parameters for DH
    	try {
        	AlgorithmParameterGenerator paramGen = AlgorithmParameterGenerator.getInstance("DH");
        	paramGen.init(size);

        	AlgorithmParameters params = paramGen.generateParameters();
        	DHParameterSpec dhSpec = (DHParameterSpec)params.getParameterSpec(DHParameterSpec.class);

        	KeyPairGenerator kpGen = KeyPairGenerator.getInstance("DH");
        	kpGen.initialize(dhSpec); //genderate keypair
        	return kpGen.generateKeyPair();
    	} catch (NoSuchAlgorithmException|InvalidParameterSpecException|InvalidAlgorithmParameterException e) {
        	e.printStackTrace();
        	return null;
    	}
	}

	/**
 	* generateKeyPair
 	*
 	* Generates a NEW keypair for DH using existing parameters.
 	*/
	public static KeyPair generateKeyPair (DHParameterSpec params) {
    	try {
        	KeyPairGenerator gen = KeyPairGenerator.getInstance("DH");
        	gen.initialize(params);
        	return gen.generateKeyPair(); //return dh key
    	} catch(NoSuchAlgorithmException|InvalidAlgorithmParameterException e) {
        	e.printStackTrace();
        	return null;
    	}
	}

	/**
 	* saveKeyPair
 	*
 	* Saves the given keypair to a file pair (filename for private, filename.pub for public)
 	*/
	public static void saveKeyPair (KeyPair kp, String filename) throws IOException {
    	writeFile(filename, kp.getPrivate().getEncoded());
    	writeFile(filename + ".pub", kp.getPublic().getEncoded());
	}

	/**
 	* generateSecretKey
 	*
 	* Generates a shared SecretKey using a (Private,Public) keypair
 	*/
	public static SecretKey generateSecretKey (PrivateKey privateKey, PublicKey publicKey) {
    	try {
        	KeyAgreement ka = KeyAgreement.getInstance("DiffieHellman");
        	ka.init(privateKey); //one user private key
        	ka.doPhase(publicKey, true); // the other user public key
        	return ka.generateSecret("AES");
    	} catch(NoSuchAlgorithmException|InvalidKeyException e) {
        	e.printStackTrace();
        	return null;
    	}
	}

	/**
 	* encrypt
 	*
 	* Encrypts a byte-string of plaintext using the given SecretKey
 	*/
	public static byte[] encrypt (byte[] plaintext, SecretKey key) {
    	try {

        	Cipher c = Cipher.getInstance("AES/ECB/PKCS5Padding"); // using build in Aes,ecb,pad
        	c.init(Cipher.ENCRYPT_MODE, key);
        	return c.doFinal(plaintext);
    	} catch (NoSuchAlgorithmException|InvalidKeyException|IllegalBlockSizeException|
             	NoSuchPaddingException|BadPaddingException e) {
        	e.printStackTrace();
        	return null;
    	}
	}

	/**
 	* decrypt
 	*
 	* Decrypts a byte string using the given SecretKey
 	*/
	public static byte[] decrypt (byte[] ciphertext, SecretKey key) {
    	try {
        	Cipher c = Cipher.getInstance("AES/ECB/PKCS5Padding");
        	c.init(Cipher.DECRYPT_MODE, key);
        	return c.doFinal(ciphertext);
    	} catch (NoSuchAlgorithmException|InvalidKeyException|IllegalBlockSizeException|
             	NoSuchPaddingException|BadPaddingException e) {
        	e.printStackTrace();
        	return null;
    	}
	}

	/**
 	* readFile
 	*
 	* Reads a file and returns its string representation.
 	*/
	public static String readFile(String path, Charset encoding) throws IOException {
        	//byte[] encoded = Files.readAllBytes(Paths.get(path));
        	byte[] encoded = Files.readAllBytes(Paths.get(path));
        	return new String(encoded, encoding);
	}

	/**
 	* readByteFile
 	*
 	* Reads a file and returns its byte[] representation.
 	*/
	public static byte[] readByteFile (String filename) throws IOException {
    	return Files.readAllBytes(Paths.get(filename));
	}
    
	/**
 	* writeFile
 	*
 	* Writes the given byte[] to a file
 	*/
	public static void writeFile(String filename, byte[] contents) throws IOException {
    	FileOutputStream fos = new FileOutputStream(new File(filename));
    	fos.write(contents);
    	fos.close();
	}
 
}//end of class
