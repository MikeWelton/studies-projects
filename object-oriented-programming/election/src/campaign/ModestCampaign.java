package campaign;

import main.District;
import main.Party;

/* Kampania skromna */
public class ModestCampaign extends CampaignStrategy {
	public ModestCampaign(char name) {
		super(name);
	}
	
	@Override
	protected void decideIfBetter(int[] action, District district, Party party) {
		if(countCost(action, district) < countCost(this.chosenAction, this.chosenDistrict)) {
			this.chosenAction = action;
			this.chosenDistrict = district;
		}
	}
}
