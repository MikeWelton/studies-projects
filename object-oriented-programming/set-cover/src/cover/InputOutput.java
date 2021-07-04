package cover;

import java.util.Scanner;

public class InputOutput {
	
	/*Funkcja do czytania danych z wejścia. */
	public void readInput(FamilyOfSets family) {
		Scanner scanner = new Scanner(System.in);
		while (scanner.hasNextInt()) {
			this.readNextCommand(scanner, family);
		}
	}
	
	/*Zaczyna czytać następną linijkę i na podstawie pierwszego znaku decyduje czy
	* wprowadzono nowy zbiór, zbiór pusty czy pelecenie znalezienia pokrycia zbioru. */
	private void readNextCommand(Scanner scanner, FamilyOfSets family) {
		int x = scanner.nextInt();
		if(x > 0) {
			Set newSet = readSetContents(x, scanner);
			family.addSet(newSet);
		}
		else if(x < 0) {
			int range = -x;
			int numberOfSolution = scanner.nextInt();
			Set setToCover = new Set(range);
			Solution solution = Solution.selectAlgorithm(numberOfSolution);
			solution.solveProblem(setToCover, family);
			printAnswer(solution);
		}
		else {
			Set newSet = new EmptySet();
			family.addSet(newSet);
		}
	}
	
	/*Pisze odpowiedź otrzymaną od roziązania.*/
	private void printAnswer(Solution solution) {
		String answer = solution.toString();
		System.out.println(answer);
	}
	
	/*Czyta kolejne zbiory/elementy zbiorów aż do wystąpienia znaku 0. */
	private Set readSetContents(int firstElement, Scanner scanner) {
		Set set = new Set();
		int first = firstElement, prev = firstElement, current;
		do {
			current = scanner.nextInt();
			if(current >= 0) {
				if(prev > 0) {
					set.addElement(prev);
				}
				else if(prev < 0 && first > 0) {
					InfiniteSequence sequence = new InfiniteSequence(first, -prev);
					set.addSequence(sequence);
				}
			}
			else {//(current < 0)
				if(prev < 0) {
					if(first <= -current) { //sprawdzamy czy skończony ciąg ma limit nie mniejszy niż pierwszy wyraz
						FiniteSequence sequence = new FiniteSequence(first, -prev, -current);
						set.addSequence(sequence);
					}
				}
			}
			first = prev;
			prev = current;
		} while(current != 0);
		return set;
	}
}
