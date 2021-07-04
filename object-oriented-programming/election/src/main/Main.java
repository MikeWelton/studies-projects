package main;

import method.MethodForAllocatingSeats;

import java.io.File;
import java.io.FileNotFoundException;
import java.util.List;

import static java.lang.System.exit;

public class Main {

	/* Główna funkcja, przeprowadzająca cały proces wyborczy. Na początku czyta dane
	wejściowe z pliku, potem łączy okręgi, przeprowadza kampanię oraz głosowanie i
	liczy glosy.
	Na końcu przydziela mandaty każdą z trzech metod i wypisuje wyniki wyborów.
	Uwaga! Dane wejściowe powinny znajdować się w pliku przekazywanym jako jedyny
	argument dla programu. W przeciwnym przypadku program kończy działanie.
	W sytuacji, gdy podany plik nie istnieje program wyrzuca wyjątek.
	 */
    public static void main(String[] args) {
    	if(args.length != 1) {
			System.out.println("Wrong number of arguments.\n" +
					"Correct usage: <program> <input-file>");
    		exit(1);
		}
		Bajtocja bajtocja = new Bajtocja();
		Input input = new Input();
		
		File file = new File(args[0]);
		try {
			input.readInput(bajtocja, file);
		} catch (FileNotFoundException e) {
			System.out.println(e.getMessage());
			exit(1);
		}
	
		bajtocja.mergeDistricts();
	
		bajtocja.runCampaign();
	
		Party[] parties = bajtocja.getParties();
		List<District> districts = bajtocja.getDistricts();
		ElectionCommission electionCommission = new ElectionCommission(parties);
		electionCommission.conductVoting(districts);
		electionCommission.countVotes(districts);
		
		for(int i = 0; i < 3; ++i) {
			electionCommission.resetMandates();
			MethodForAllocatingSeats method = MethodForAllocatingSeats.chooseMethod(i, parties);
			allocateSeatsAndShowResults(electionCommission, method, districts);
		}
    }
    
    /* Rozdziela mandaty poszczególnym partiom i wypisuje wyniki wyborów. */
    public static void allocateSeatsAndShowResults(ElectionCommission electionCommission,
				MethodForAllocatingSeats method, List<District> districts) {
    
		electionCommission.allocateSeats(method, districts);
		electionCommission.showResults(method, districts);
	}
}
