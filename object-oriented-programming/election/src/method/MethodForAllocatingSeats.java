package method;

import main.District;
import main.Party;

import java.util.LinkedHashMap;
import java.util.Map;

public abstract class MethodForAllocatingSeats {
	protected String name;
	protected Map<String, Integer> mandatesPerParty;
	
	public MethodForAllocatingSeats(Party[] parties) {
		this.mandatesPerParty = new LinkedHashMap<>();
		for(Party party: parties) {
			this.mandatesPerParty.put(party.getName(), 0);
		}
	}
	
	/* Zwraca wybraną metodę głosowania. */
	public static MethodForAllocatingSeats chooseMethod(int num, Party[] parties) {
		switch(num) {
			case 0:
				return new DHondtMethod(parties);
			case 1:
				return new SainteLagueMethod(parties);
			default: //case 2
				return new HareNiemeyerMethod(parties);
		}
	}
	
	public abstract void allocateSeats(District district);
	
	public String toString() {
		return this.name;
	}
	
	/* Przepisuje wartości z mapy to tablicy intów. */
	protected int[] mapValuesToArray(Map<String, Integer> map) {
		int[] array = new int[map.size()];
		int i = 0;
		for(Map.Entry<String, Integer> entry: map.entrySet()) {
			array[i] = entry.getValue();
			++i;
		}
		return array;
	}
	
	/* Dodaje po kolei wartości z tablicy intów do mapy. */
	protected void arrayToMapValues(int[] array) {
		int i = 0;
		for(Map.Entry<String, Integer> entry: this.mandatesPerParty.entrySet()) {
			this.mandatesPerParty.put(entry.getKey(), array[i]);
			++i;
		}
	}
}
