package hw1;

import hw1.LibraryBook;
import java.io.FileWriter;
import java.io.IOException;

public final class Borrowed implements LBState{
	
	private static Borrowed instance = null;
	
	public synchronized static Borrowed getInst() {
		if(instance == null) {
			instance = new Borrowed();
		}
		return instance;
	}
	
	private Borrowed() {
		try {
			// a second parameter is needed to support append mode
			LibraryBook.writer = new FileWriter(LibraryBook.file , true);
			LibraryBook.writer.append(this + " Instance Created\n");
			LibraryBook.writer.close();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
//		System.out.println(this + " Instance Created");
	}

	@Override
	public void returnIt(LibraryBook book) throws BadOperationException {
		book.setState(GotBack.getInst());
		try {
			// a second parameter is needed to support append mode
			LibraryBook.writer = new FileWriter(LibraryBook.file , true);
			LibraryBook.writer.append("Leaving state " + this + " for State " + book.getState() + "\n");
			LibraryBook.writer.close();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
//		System.out.println("Leaving state " + this + " for State " + book.getState());
	}

	@Override
	public void issue(LibraryBook book) throws BadOperationException {
		throw new BadOperationException("Can't use issue in " + this + " state");	
	}

	@Override
	public void extend(LibraryBook book) throws BadOperationException {
		try {
			// a second parameter is needed to support append mode
			LibraryBook.writer = new FileWriter(LibraryBook.file , true);
			LibraryBook.writer.append("Leaving State " + this + " for State " + this + "\n");
			LibraryBook.writer.close();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
//		System.out.println("Leaving State " + this + " for State " + this);	
	}

	@Override
	public void shelf(LibraryBook book) throws BadOperationException {
		throw new BadOperationException("Can't use shelf in " + this + " state.");
	}
	
	@Override
	public String toString() {
		return "Borrowed";
	}

}
