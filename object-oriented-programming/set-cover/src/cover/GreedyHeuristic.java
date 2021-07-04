package cover;

import java.util.ArrayList;
import java.util.Collections;

public class GreedyHeuristic extends Solution {
	
	/* Funkcja, zgodnie ze specyfikacją w kolejnych krokach wybiera z rodziny zbiorów zbiór,
	* w którym jest największa liczba elementów pokrywanego zbioru,
	*  nie należących do żadnego z dotąd wybranych zbiorów. */
	@Override
	protected void solveProblem(Set setToCover, FamilyOfSets family) {
		ArrayList<Set> setsFromFamily = family.getAllSets();
		ArrayList<Integer> coveringSets = new ArrayList<Integer>();
		boolean[] wasUsedBefore = new boolean[setsFromFamily.size() + 1];
		/*tablica, w której oznaczamy czy zbiór z rodziny
		został już użyty do pokrycia zbioru */
		
		int sizeOfBiggestCommonPart;
		Set setWithMostCommonNumbers;
		
		do {
			sizeOfBiggestCommonPart = 0;
			setWithMostCommonNumbers = null;
			for(Set set: setsFromFamily) {
				int commonPartSize = set.sizeOfCommonPart(setToCover);
				if(commonPartSize > sizeOfBiggestCommonPart
				&& !wasUsedBefore[set.getIndex()]) {
					sizeOfBiggestCommonPart = commonPartSize;
					setWithMostCommonNumbers = set;
				}
			}
			if(sizeOfBiggestCommonPart > 0) {
				int setNumber = setWithMostCommonNumbers.getIndex();
				wasUsedBefore[setNumber] = true;
				coveringSets.add(setNumber);
				setWithMostCommonNumbers.deleteCommonPartFrom(setToCover);
			}
		} while(sizeOfBiggestCommonPart > 0);
		
		if(setToCover.isEmpty()) {
			Collections.sort(coveringSets);
			this.listOfSets = coveringSets;
		}
		else {
			this.listOfSets.add(0);
		}
	}
}
