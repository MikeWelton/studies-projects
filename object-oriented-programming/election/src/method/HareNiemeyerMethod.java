package method;

import main.District;
import main.Party;

import java.util.Map;

public class HareNiemeyerMethod extends MethodForAllocatingSeats {
	public HareNiemeyerMethod(Party[] parties) {
		super(parties);
		this.name = "Hare–Niemeyer method";
	}
	
	@Override
	public void allocateSeats(District district) {
		int allVotes = district.getNumOfVoters();
		int numOfMandates = district.getNumOfMandates();
		int numOfParties = this.mandatesPerParty.size();
		int[] mandates = new int[numOfParties];
		float[] remainders = new float[numOfParties];
		int remainingMandates = numOfMandates;
		
		/* Najpierw zapisujemy osobno części całkowite i reszty. */
		int i = 0;
		for(Map.Entry<String, Integer> entry: district.getVotesPerParty().entrySet()) {
			int receivedVotes = entry.getValue();
			mandates[i] = (receivedVotes * numOfMandates) / allVotes;
			remainders[i] = ((receivedVotes * numOfMandates) / (float)allVotes) - mandates[i];
			remainingMandates -= mandates[i];
			
			++i;
		}
		
		/* Dla wszystkich nieprzyznanych mandatów szukamy numeru w tablicy, pod ktorym
		* znajduje się największa reszta. Pod ten numer w tablicy mandatów dodajemy
		* jeden mandat, a samą resztę zerujemy, żeby nie przeszkadzała przy kolejnym
		* przeszukiwaniu. */
		for(i = 0; i < remainingMandates; ++i) {
			int leader = 0;
			for(int j = 0; j < remainders.length; ++j) {
				if(remainders[j] > remainders[leader]) {
					leader = j;
				}
			}
			++mandates[leader];
			remainders[leader] = 0;
		}
		
		arrayToMapValues(mandates);
		district.setMandatesPerParty(this.mandatesPerParty);
	}
}
