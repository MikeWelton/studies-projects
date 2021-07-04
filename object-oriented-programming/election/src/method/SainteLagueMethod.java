package method;

import main.District;
import main.Party;

public class SainteLagueMethod extends MethodForAllocatingSeats {
	public SainteLagueMethod(Party[] parties) {
		super(parties);
		this.name = "Sainte-Laguë method";
	}
	
	@Override
	public void allocateSeats(District district) {
		int numOfMandates = district.getNumOfMandates();
		int numOfParties = this.mandatesPerParty.size();
		int[] votes = mapValuesToArray(district.getVotesPerParty());
		int[] mandates = new int[numOfParties];
		
		for(int i = 0; i < numOfMandates; ++i) {
			int leader = 0;
			
			for(int j = 0; j < numOfParties; ++j) {
				if(votes[j] / (2 * mandates[j] + 1) > votes[leader] / (2 * mandates[leader] + 1)) {
					leader = j;
				}
			}
			
			++mandates[leader];
		}
		
		arrayToMapValues(mandates);
		district.setMandatesPerParty(this.mandatesPerParty);
	}
}
