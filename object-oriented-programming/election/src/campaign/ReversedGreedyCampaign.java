package campaign;

import main.District;
import main.Party;

/* Kampania własny. Kampania odwrotna do zachłannej, to znaczy wybierająca zawsze
* działanie, które w najmniejszym stopniu zmieni sumę ważoną kandydatów wśród wyborców. */
public class ReversedGreedyCampaign extends CampaignStrategy {
	private long influence;
	public ReversedGreedyCampaign(char name) {
		super(name);
	}
	
	@Override
	protected void setInfluenceValue(int[] action, District district, Party party) {
		this.influence = calculateInfluence(action, district, party);
	}
	
	@Override
	protected void decideIfBetter(int[] action, District district, Party party) {
		if(calculateInfluence(action, district, party) < this.influence) {
			this.chosenAction = action;
			this.chosenDistrict = district;
		}
	}
}
