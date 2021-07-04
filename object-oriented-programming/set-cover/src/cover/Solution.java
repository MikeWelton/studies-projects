package cover;

import java.util.*;

public abstract class Solution {
	protected ArrayList<Integer> listOfSets;
	
	public Solution() {
		this.listOfSets = new ArrayList<Integer>();
	}
	
	/*Funkcja rozwiązująca problem pokrycia danego zbioru daną rodziną.
	* Jest implementowana osobno przez każde z rozwiązań.*/
	protected abstract void solveProblem(Set setToCover, FamilyOfSets family);
	
	/*Wybiera algorytm w zależności od numeru (otrzymanego na wejściu).*/
	public static Solution selectAlgorithm(int numberOfSolution) {
		switch(numberOfSolution) {
			case 1:
				return new ExactAlgorithm();
			case 2:
				return new GreedyHeuristic();
			default: //case 3
				return new NaiveHeuristic();
		}
	}
	
	/*Porównuje dany ciąg numerów z numerami zbiorów zapisanymi w atrybucie klasy.
	Zwraca true, jeśli nowy ciąg jest krótszy lub tej samej długości, ale
	wsześniejszy leksykograficznie od ciągu w atrybucie, a false w przeciwnym wypadku. */
	protected boolean compare(ArrayList<Integer> checkedList) {
		if(checkedList.isEmpty()) {
			return false;
		}
		if(this.listOfSets.isEmpty() || checkedList.size() < this.listOfSets.size()) {
			return true;
		}
		Iterator<Integer> i = checkedList.iterator();
		Iterator<Integer> j = this.listOfSets.iterator();
		int x, y;
		while(i.hasNext() && j.hasNext()) {
			x = i.next();
			y = j.next();
			if(x < y) {
				return true;
			}
			else if(x > y) {
				return false;
			}
		}
		if(i.hasNext()) {
			return false;
		}
		else {//jeśli obydwa ciągi się zakończyły to są równe i zwracamy false
			return j.hasNext();
		}
	}
	
	/*Inna wersja tej samej funkcji co wyżej, różniąca się tym, że przyjmuje jako
	* argument tablicę. */
	protected boolean compare(int[] checkedArray) {
		ArrayList<Integer> checkedList = new ArrayList<Integer>();
		for (int x : checkedArray) {
			checkedList.add(x);
		}
		return compare(checkedList);
	}
	
	/*Zwraca listę zbiorów będących rozwiązaniem problemu pokrycia zbioru w formie napisu,
	* który można wypisać na wyjście. */
	public String toString() {
		String ret = "";
		int size = this.listOfSets.size();
		for(int i = 0; i < size; i++) {
			ret += listOfSets.get(i);
			if(i != size - 1) {
				ret += " ";
			}
		}
		return ret;
	}
}
