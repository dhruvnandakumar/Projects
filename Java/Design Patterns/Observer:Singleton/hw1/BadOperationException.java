package hw1;

import java.io.FileWriter;
import java.io.IOException;

public class BadOperationException extends Exception {
	private static final long serialVersionUID = 8043957391935983504L;

	public BadOperationException(String message) {
		super(message);
		try {
			// a second parameter is needed to support append mode
			LibraryBook.writer = new FileWriter(LibraryBook.file , true);
			LibraryBook.writer.append("hw1.BadOperationException: " + message + "\n");
			LibraryBook.writer.close();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}
}
