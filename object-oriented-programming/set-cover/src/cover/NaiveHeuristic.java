package cover;

import java.util.ArrayList;
import java.util.TreeSet;

public class NaiveHeuristic extends Solution {
	
	/*Funkcja, zgodnie ze specyfikacją, przechodzi wszystkie zbiory od początku i
	* dodaje numer zbioru do rozwiąznia, jeśli ten zawiera choć jeden wspólny element ze
	* zbiorem do pokrycie, którego nie zawierał żaden poprzedni zbiór.*/
	@Override
	protected void solveProblem(Set setToCover, FamilyOfSets family) {
		ArrayList<Set> setsFromFamily = family.getAllSets();
		ArrayList<Integer> coveringSets = new ArrayList<Integer>();
		TreeSet<Integer> setToCoverElements = setToCover.getElements();
		for(Set set: setsFromFamily) {
			int commonPartSize = set.getCommonPartSizeAndDeleteItFrom(setToCoverElements);
			if(commonPartSize > 0) {
				coveringSets.add(set.getIndex());
			}
		}
		if(setToCover.isEmpty()) {
			this.listOfSets = coveringSets;
		}
		else {
			this.listOfSets.add(0);
		}
	}
}
