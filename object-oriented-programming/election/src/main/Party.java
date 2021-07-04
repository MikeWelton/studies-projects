package main;

import campaign.CampaignStrategy;
import voter.Voter;

import java.util.List;

public class Party {
	private String name;
	private int budget;
	private CampaignStrategy strategy;
	
	
	public String getName() {
		return this.name;
	}
	
	public CampaignStrategy getStrategy() {
		return this.strategy;
	}
	
	
	public void setName(String name) {
		this.name = name;
	}
	
	public void setBudget(int budget) {
		this.budget = budget;
	}
	
	public void setStrategy(char strategy) {
		this.strategy = CampaignStrategy.chooseMethod(strategy);
	}
	
	
	
	public void runCampaign(int[][] possibleActions, List<District> districts) {
		while(true) { //partia wykonuje akcje tak długo aż nie skończą się pieniądze
			this.strategy.setChosenAction(null);
			this.strategy.setChosenDistrict(null);
			//na początku nie ma wybranej żadnej akcji, ani okręgu
			
			for(District district: districts) {
				checkActions(possibleActions, district);
			}
			
			//jeśli nie przypisano żadnej akcji to oznacza, że partia nie miała pieniędzy
			if(!this.strategy.hasAction()) {
				break;
			}
			
			realizeAction(this.strategy.getChosenDistrict());
		}
	}
	
	private void checkActions(int[][] possibleActions, District district) {
		for(int[] action: possibleActions) {
			if(this.strategy.countCost(action, district) <= budget) {
				this.strategy.reviewAction(action, district, this);
			}
		}
	}
	
	public void realizeAction(District district) {
		int[] action = this.strategy.getChosenAction();
		this.budget -= this.strategy.countCost(action, district);
		for(Voter voter: district.getVoters()) {
			voter.influenceOpinion(action);
		}
	}
}
