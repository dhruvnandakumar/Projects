package hw1;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.*;

public class LibraryBook implements Subject{
	
	private List<Observer> observers;
	private LBState currentState;
	private String name;
	static File file = new File("Q2&3Log.txt");
	static FileWriter writer;

	public LibraryBook(String n) {
		// Create the file
		try {
			if (file.createNewFile()) {
			    System.out.println("Q2&3Log.txt is created!");
			} else {
			    System.out.println("Q2&3Log.txt already exists.");
			}
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		
		name = n;
		observers = new ArrayList<Observer>();
		currentState = OnShelf.getInst();
	}
	
	@Override
	public void attach(Observer obs) {
		try {
			// a second parameter is needed to support append mode
			writer = new FileWriter(file, true);
			writer.append(obs + " is now watching " + name + "\n");
			writer.close();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
//		System.out.println(obs + " is now watching " + name);
		observers.add(obs);
	}
	
	@Override
	public void detach(Observer obs) {
		if(observers.contains(obs)) {
			try {
				// a second parameter is needed to support append mode
				writer = new FileWriter(file, true);
				writer.append(obs + " is no longer watching " + name + "\n");
				writer.close();
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
//			System.out.println(obs + " is no longer watching " + name);
			observers.remove(obs);
		}
	}
	
	public void Notify() {
		for(Observer obs:observers) {
			obs.Update(this);
		}
	}
	
	public void returnIt() {
		try {
			currentState.returnIt(this);
			Notify();
		}
		catch(BadOperationException e) {
			System.out.println(e);
		}
	}
	
	public void issue() {
		try {
			currentState.issue(this);
			Notify();
		}
		catch(BadOperationException e) {
			System.out.println(e);
		}
	}
	
	public void extend() {
		try {
			currentState.extend(this);
			//We don't call Notify() when moving to the same state again.
		}
		catch(BadOperationException e) {
			System.out.println(e);
		}
	}
	
	public void shelf() {
		try {
			currentState.shelf(this);
			Notify();
		}
		catch(BadOperationException e) {
			System.out.println(e);
		}
	}
	
	public void setState(LBState newState) {
		currentState = newState;
	}
	
	@Override
	public String getState() {
		return currentState.toString();
	}
	
	@Override
	public boolean equals(Object obj) {
		if(obj == this) {
			return true;
		}
		
		if(!(obj instanceof LibraryBook)) {
			return false;
		}
		
		LibraryBook other = (LibraryBook)obj;
		
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
