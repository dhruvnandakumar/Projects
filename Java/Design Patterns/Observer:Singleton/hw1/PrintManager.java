package hw1;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;

/**
 * This example is not accounting for serialization
 * and is not a solution that can be sub-classed.
 * It is thread safe, but has potentially poor performance.
 * It implements a few more conditions than were strictly 
 * required for the assignment.
 * 
 * Notes on the trickiness of implementing Singletons with 
 * Java in real world situations can be found here:
 * https://www.javaworld.com/article/2073352/core-java/simply-singleton.html
 *
 */
public final class PrintManager {
	
	private static PrintManager instance = null;
	private static File file = new File("Q1Log.txt");
	private static FileWriter writer;
	
	public static PrintManager ThePrintManager() {
		if(instance == null) {
			instance = new PrintManager();
		}
		else {
			try {
				// a second parameter is needed to support append mode
				writer = new FileWriter(file, true);
				writer.append("Previously Created instance returned\n");
				writer.close();
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		}
		return instance;
	}
	
	private PrintManager() {	  
		// Create the file
		try {
			if (file.createNewFile()) {
			    System.out.println("Q1Log.txt is created!");
			} else {
			    System.out.println("Q1Log.txt already exists.");
			}
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		
		// Write Content
		try {
			writer = new FileWriter(file);
			writer.write("Instance Created\n");
			writer.close();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}	
	}
	
}
