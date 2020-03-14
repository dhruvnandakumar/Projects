package hw1;

public interface Subject {
	public void attach(Observer obs);
	public void detach(Observer obs);
	public void Notify();
	public String getState();

}
