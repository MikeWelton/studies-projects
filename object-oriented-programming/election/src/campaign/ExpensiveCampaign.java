package campaign;

import main.District;
import main.Party;

/* Kampania z rozmachem */
public class ExpensiveCampaign extends CampaignStrategy {
	public ExpensiveCampaign(char name) {
		super(name);
	}
	
	@Override
	protected void decideIfBetter(int[] action, District district, Party party) {
		if(countCost(action, district) > countCost(this.chosenAction, this.chosenDistrict)) {
			this.chosenAction = action;
			this.chosenDistrict = district;
		}
	}
}
