package hw1;

import java.io.FileWriter;
import java.io.IOException;

import hw1.LibraryBook;

public final class GotBack implements LBState{
	
	private static GotBack instance = null;
	
	public synchronized static GotBack getInst() {
		if(instance == null) {
			instance = new GotBack();
		}
		return instance;
	}
	
	private GotBack() {
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
		throw new BadOperationException("Can't use issue in " + this + " state");
	}

	@Override
	public void extend(LibraryBook book) throws BadOperationException {
		throw new BadOperationException("Can't use extend in " + this + " state");
	}

	@Override
	public void shelf(LibraryBook book) throws BadOperationException {
		book.setState(OnShelf.getInst());
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
	public String toString() {
		return "Gotback";
	}

}
