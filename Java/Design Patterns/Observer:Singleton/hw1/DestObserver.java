package hw1;

import hw1.LibraryBook;
import java.io.FileWriter;
import java.io.IOException;

public class DestObserver implements Observer{
	
	private String name;
	
	public DestObserver(String n) {
		name = n;
	}

	@Override
	public void Update(Subject o) {
		try {
			// a second parameter is needed to support append mode
			LibraryBook.writer = new FileWriter(LibraryBook.file, true);
			LibraryBook.writer.append(name + " OBSERVED " + o + " REACHING STATE: " + o.getState() + "\n");
			LibraryBook.writer.close();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
//		System.out.println(name + " OBSERVED " + o + " REACHING STATE: " + o.getState());
	}
	
	@Override
	public boolean equals(Object obj) {
		if(obj == this) {
			return true;
		}
		
		if(!(obj instanceof DestObserver)) {
			return false;
		}
		
		DestObserver other = (DestObserver)obj;
		
		return other.name.equals(this.name);
	}
	
	@Override
	public int hashCode() {
		return name.hashCode();
	}
	
	@Override
	public String toString() {
		return name;
	}


}
