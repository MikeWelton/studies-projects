package campaign;

import main.District;
import main.Party;

/* Kampania zachłanna */
public class GreedyCampaign extends CampaignStrategy {
	private long influence;
	public GreedyCampaign(char name) {
		super(name);
	}
	
	@Override
	protected void setInfluenceValue(int[] action, District district, Party party) {
		this.influence = calculateInfluence(action, district, party);
	}
	
	@Override
	protected void decideIfBetter(int[] action, District district, Party party) {
		if(calculateInfluence(action, district, party) > this.influence) {
			this.chosenAction = action;
			this.chosenDistrict = district;
		}
	}
}
