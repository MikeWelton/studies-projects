package main;

import method.MethodForAllocatingSeats;

import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

public class ElectionCommission {
	private Map<String, Integer> mandatesPerParty;
	
	public ElectionCommission(Party[] parties) {
		this.mandatesPerParty = new LinkedHashMap<>();
		for(Party party: parties) {
			this.mandatesPerParty.put(party.getName(), 0);
		}
	}
	
	/* Przeprowadza głosowanie we wszystkich okręgach. */
	public void conductVoting(List<District> districts) {
		for(District district: districts) {
			district.conductVoting();
		}
	}
	
	/* Liczy głosy we wszystkich okręgach. */
	public void countVotes(List<District> districts) {
		for(District district: districts) {
			district.countVotes();
		}
	}
	
	/* Resetuje podział mandatów zapisany w atrybucie. Konieczne, gdy przy użyciu tej
	* samej komisji wyborczej chcemy policzyć podział mandatów wszystkimi trzema sposobami.
	*/
	public void resetMandates() {
		for(Map.Entry<String, Integer> entry: this.mandatesPerParty.entrySet()) {
			this.mandatesPerParty.put(entry.getKey(), 0);
		}
	}
	
	/*Funkcja, w pętli, najpierw rozdziela mandaty w okręgu wyborczym,
	* następnie pobiera je i dodaje mandaty z okręgu do atrybutu zapisującego
	* ogolnokrajowy podział mandatów.*/
	public void allocateSeats(MethodForAllocatingSeats method, List<District> districts) {
		for(District district: districts) {
			method.allocateSeats(district);
			Map<String, Integer> mandatesFromDistrict = district.getMandates();
			
			for(Map.Entry<String, Integer> entry: mandatesFromDistrict.entrySet()) {
				String partyName = entry.getKey();
				int mandates = entry.getValue();
				this.mandatesPerParty.put(partyName, mandatesPerParty.get(partyName) + mandates);
			}
		}
	}
	
	/* Wypisuje nazwę użytej metody, następnie informacje ze wszystkich okregów, na koniec
	podsumowanie liczby mandatów dla każdej z partii.
	 */
	public void showResults(MethodForAllocatingSeats method, List<District> districts) {
		System.out.println(method.toString());
		
		for(District district: districts) {
			district.showResults();
		}
		
		for(Map.Entry<String, Integer> entry: this.mandatesPerParty.entrySet()) {
			System.out.println(entry.getKey() + " " + entry.getValue().toString());
		}
	}
}
