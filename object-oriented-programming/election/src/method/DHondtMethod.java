package method;

import main.District;
import main.Party;

public class DHondtMethod extends MethodForAllocatingSeats {
	public DHondtMethod(Party[] parties) {
		super(parties);
		this.name = "D'Hondt method";
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
				if(votes[j] / (mandates[j] + 1) > votes[leader] / (mandates[leader] + 1)) {
					leader = j;
				}
			}
			
			++mandates[leader];
		}
		
		arrayToMapValues(mandates);
		district.setMandatesPerParty(this.mandatesPerParty);
	}
}
