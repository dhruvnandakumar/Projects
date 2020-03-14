package hw1;

public interface LBState {

	public void returnIt(LibraryBook book) throws BadOperationException;
	public void issue(LibraryBook book) throws BadOperationException;
	public void extend(LibraryBook book) throws BadOperationException;
	public void shelf(LibraryBook book) throws BadOperationException;
	
}
