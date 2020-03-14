package hw1;

import java.io.FileWriter;
import java.io.IOException;

import hw1.LibraryBook;

public final class OnShelf implements LBState{
	
	private static OnShelf instance = null;
	
	public synchronized static OnShelf getInst() {
		if(instance == null) {
			instance = new OnShelf();
		}
		return instance;
	}
	
	private OnShelf() {
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
		throw new BadOperationException("Can't use returnIt in " + this + " state");
		
	}

	@Override
	public void issue(LibraryBook book) throws BadOperationException {
		book.setState(Borrowed.getInst());
		try {
			// a second parameter is needed to support append mode
			LibraryBook.writer = new FileWriter(LibraryBook.file , true);
			LibraryBook.writer.append("Leaving state " + this + " for State " + book.getState() + "\n");
			LibraryBook.writer.close();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
//		System.out.println("Leaving State " + this + " for State " + book.getState());
	}

	@Override
	public void extend(LibraryBook book) throws BadOperationException {
		throw new BadOperationException("Can't use extend in " + this + " state");

	}

	@Override
	public void shelf(LibraryBook book) throws BadOperationException {
		throw new BadOperationException("Can't use shelf in " + this + " state");
	}
	
	@Override
	public String toString() {
		return "OnShelf";
	}

}
