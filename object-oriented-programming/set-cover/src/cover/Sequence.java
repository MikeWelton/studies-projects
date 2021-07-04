package cover;

import java.util.TreeSet;

public abstract class Sequence {
	protected int firstElement;
	protected int difference;
	
	public Sequence(int firstElement, int difference) {
		this.firstElement = firstElement;
		this.difference = difference;
	}
	
	/*Implementacje poniższych funkcji dla podklas klasy Sequence różnią się tylko
	* wyborem ostatniego elementu, do którego wykonywana jest pętla.
	* W przypadku ciągu nieskończonego będzie to największy element ze zbioru do pokrycia.
	* W przypadku ciągu skończonego limitem będzie mniejszy z pary
	* (największy element zbioru do pokrycia, górne ograniczenie ciągu). */
	
	/*Zwraca liczbę wspólnych elementów tego ciągu i zbioru do pokrycia.*/
	public abstract int numberOfCommonElements(TreeSet<Integer> elementsOfCoveredSet);
	
	/*Usuwa wspólne elementy ze zbioru do pokrycia.*/
	public abstract void deleteCommonPartFrom(TreeSet<Integer> elementsOfCoveredSet);
	
	/*Zwraca drzewo zawierające elementy części wspólnej tego ciągu z pokrywanym zbiorem.*/
	public abstract TreeSet<Integer> getCommonPart(TreeSet<Integer> elementsOfCoveredSet);
}
