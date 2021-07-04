package cover;

import java.util.ArrayList;

public class ExactAlgorithm extends Solution {
	/*Rozwiązuje problem generując wszystkie możliwe kombinacje zbiorów. Ponieważ kombinacje
	* są generowane od najkrótszej, funkcja opuszcza pętle (za pomocą break), gdy
	* została znaleziona pasująca kombinacja.*/
	@Override
	protected void solveProblem(Set setToCover, FamilyOfSets family) {
		int n = family.size();
		
		for(int k = 1; k <= n; ++k) {
			generateCombinations(n, k, setToCover, family);
			if(!this.listOfSets.isEmpty()) {
				break;
			}
		}
		if(this.listOfSets.isEmpty()) {
			this.listOfSets.add(0);
		}
	}
	
	/*Funkcja najpierw tworzy pierwszą leksykograficznie k elementową kombinację ze
	* zbioru od 1 od n, następnie generuje wszystkie kolejne k elementowe kombinacje.
	* Ponieważ kombinacje generowane są w porządku leksykograficznym, funkcja wykonuje
	* break, jeśli znaleziono pasującą kombinację.*/
	protected void generateCombinations(int n, int k, Set setToCover, FamilyOfSets family) {
		int[] combination = new int[k];
		for(int i = 0; i < k; ++i) {
			combination[i] = i + 1;
		}
		
		int i = k - 1;
		
		while(combination[0] <= n - k + 1) {
			while(i > 0 && combination[i] == n - k + i + 1) {
				--i;
			}
			
			checkCombination(combination, setToCover, family);
			if(!this.listOfSets.isEmpty()) {
				break;
			}
			++combination[i];
			
			while(i < k - 1) {
				combination[i + 1] = combination[i] + 1;
				++i;
			}
		}
	}
	
	/*Sprawdza czy dana kombinacja numerów zbiorów jest rozwiązaniem problemu. Jeśli tak,
	* przypisuje ją na atrybut rozwiązania.*/
	private void checkCombination(int[] combination, Set setToCover, FamilyOfSets family) {
		Set setToCoverCopy = setToCover.clone();
		for(int index: combination) {
			Set checkedSet = family.getSet(index);
			
			checkedSet.deleteCommonPartFrom(setToCoverCopy);
		}
		if(setToCoverCopy.isEmpty() && this.compare(combination)) {
			ArrayList<Integer> coveringSets = new ArrayList<Integer>();
			for (int x : combination) {
				coveringSets.add(x);
			}
			this.listOfSets = coveringSets;
		}
	}
}
