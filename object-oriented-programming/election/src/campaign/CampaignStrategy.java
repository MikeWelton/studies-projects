package campaign;

import main.Candidate;
import main.District;
import main.Party;
import voter.Voter;

public abstract class CampaignStrategy {
	protected char name;
	protected int[] chosenAction;
	protected District chosenDistrict;
	
	public CampaignStrategy(char name) {
		this.name = name;
	}
	
	public static CampaignStrategy chooseMethod(char name) {
		switch(name) {
			case 'R':
				return new ExpensiveCampaign(name);
			case 'S':
				return new ModestCampaign(name);
			case 'W':
				return new ReversedGreedyCampaign(name);
			default: //case 'Z
				return new GreedyCampaign(name);
		}
	}
	
	
	public int[] getChosenAction() {
		return this.chosenAction;
	}
	
	public District getChosenDistrict() {
		return this.chosenDistrict;
		
	}
	
	
	public void setChosenAction(int[] action) {
		this.chosenAction = action;
	}
	
	public void setChosenDistrict(District chosenDistrict) {
		this.chosenDistrict = chosenDistrict;
	}
	
	
	public boolean hasAction() {
		return (this.chosenAction != null);
	}
	
	public void reviewAction(int[] action, District district, Party party) {
		if(this.chosenAction == null) {
			setInfluenceValue(action, district, party);
			this.chosenAction = action;
			this.chosenDistrict = district;
		}
		else {
			decideIfBetter(action, district, party);
		}
	}
	
	/* Każda strategia sprawdza czy podana akcja w okręgu jest lepsza od tej, którą ma
	zapisaną w atrybucie.
	 */
	protected abstract void decideIfBetter(int[] action, District district, Party party);
	
	/* Ustawia wartość łącznej zmiany sum ważonych u wyborców wszechstronnych w danym okręgu.
	Nadpisywane jedynie przez strategię zachłanną i odwrotną zachłanną, gdyż tylko one
	sprawdzają tę wartość.
	 */
	protected void setInfluenceValue(int[] action, District district, Party party) {}
	
	/* Liczy łączny wpływ jaki akcja będzie miała na wyborców wszechstronnych w danym
	okręgu.
	 */
	protected long calculateInfluence(int[] action, District district, Party party) {
		long ret = 0;
		for(Candidate candidate: district.getCandidates(party)) {
			for(Voter voter: district.getVoters()) {
				ret += voter.calculateInfluence(action, candidate);
			}
		}
		return ret;
	}
	
	/* Liczy cały koszt akcji w okręgu. */
	public int countCost(int[] action, District district) {
		int costPerVoter = costPerVoter(action);
		return costPerVoter * district.getNumOfVoters();
	}
	
	/* Liczy koszt akcji na jednego mieszkańca, czyli sumę wartości bezwzględnych
	* elementów wektora zmiany wag.
	 */
	public int costPerVoter(int[] action) {
		int ret = 0;
		for(int weightChange: action) {
			if(weightChange < 0) {
				ret += (-weightChange);
			}
			else {
				ret += weightChange;
			}
		}
		return ret;
	}
}
