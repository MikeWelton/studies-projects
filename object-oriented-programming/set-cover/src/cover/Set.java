package cover;


import java.util.ArrayList;
import java.util.TreeSet;

public class Set {
	protected int index;
	protected TreeSet<Integer> elements;
	protected ArrayList<Sequence> sequences;
	
	/*Konstruktor inicjalizujący nowy obiekt.*/
	public Set() {
		this.elements = new TreeSet<Integer>();
		this.sequences = new ArrayList<Sequence>();
	}
	
	/*Konstruktor wykorzystywany do łatwego tworzenia zbioru, który ma zostać pokryty.*/
	public Set(int range) {
		this.elements = new TreeSet<Integer>();
		for(int i = 1; i <= range; i++) {
			this.elements.add(i);
		}
		this.sequences = new ArrayList<Sequence>();
	}
	
	public boolean isEmpty() {
		return (this.elements.isEmpty() && this.sequences.isEmpty());
	}
	
	public Set clone() {
		return new Set(this.elements.size());
	}
	
	/* Dodaje jeden element do zbioru.*/
	public void addElement(int element) {
		this.elements.add(element);
	}
	
	/*Dodaje do zbioru nowy ciąg elementów.*/
	public void addSequence(Sequence sequence) {
		this.sequences.add(sequence);
	}
	
	public TreeSet<Integer> getElements() {
		return this.elements;
	}
	
	public int getIndex() {
		return this.index;
	}
	
	/*Ustawia indeks tego zbioru.*/
	public void setIndex(int index) {
		this.index = index;
	}
	
	/*Liczy rozmiar części wspólnej tego zbioru z danym zbiorem zbiorem do pokrycia.*/
	public int sizeOfCommonPart(Set setToCover) {
		if(this.isEmpty()) {
			return 0;
		}
		TreeSet<Integer> elementsOfCoveredSet = new TreeSet<Integer>(setToCover.getElements());
		TreeSet<Integer> temp = new TreeSet<Integer>(this.elements);
		int counter = 0;
		temp.retainAll(elementsOfCoveredSet);
		counter += temp.size();
		
		elementsOfCoveredSet.removeAll(temp);
		//zamiast temp może być this.elements, ale wtedy znowu sprawdzamy nieistotne elementy
		
		for(Sequence sequence: sequences) {
			counter += sequence.numberOfCommonElements(elementsOfCoveredSet);
		}
		return counter;
	}
	
	/*Usuwa część wspólną ze zbioru do pokrycia.*/
	public void deleteCommonPartFrom(Set setToCover) {
		TreeSet<Integer> elementsOfCoveredSet = setToCover.getElements();
		elementsOfCoveredSet.removeAll(this.elements);
		for(Sequence sequence: sequences) {
			sequence.deleteCommonPartFrom(elementsOfCoveredSet);
		}
	}
	
	/*Zwraca rozmiar części wspólnej tego zbioru ze zbiorem do pokrycia. Równocześnie
	* usuwa wspólne elementy ze zbioru do pokrycia. */
	public int getCommonPartSizeAndDeleteItFrom(TreeSet<Integer> elementsOfCoveredSet) {
		if(this.isEmpty() || elementsOfCoveredSet.isEmpty()) {
			return 0;
		}
		TreeSet<Integer> commonElements = new TreeSet<Integer>(this.elements);
		
		commonElements.retainAll(elementsOfCoveredSet);
		if(!commonElements.isEmpty()) {
			elementsOfCoveredSet.removeAll(commonElements);
		}
		
		for(Sequence sequence: sequences) {
			TreeSet<Integer> commonWithSequence = sequence.getCommonPart(elementsOfCoveredSet);
			commonElements.addAll(commonWithSequence);
		}
		
		return commonElements.size();
	}
}
