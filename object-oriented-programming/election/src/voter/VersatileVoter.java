package voter;

import main.Candidate;
import main.Party;

import java.util.List;
import java.util.Map;

/* Wyborca wszechstronny. */
public class VersatileVoter extends Voter {
	protected int[] attributesWeights;
	protected float chosenCandidateScore;
	
	public VersatileVoter(String name, String surname, int numOfDistrict,
						  int[] attributesWeights) {
		super(name, surname, numOfDistrict);
		this.attributesWeights = attributesWeights;
	}
	
	@Override
	public void vote(Map<Party, List<Candidate>> ticket) {
		super.iterateOverTicket(ticket);
		this.chosenCandidate.addVote();
	}
	
	@Override
	protected void decideIfBetter(Candidate candidate) {
		if(chosenCandidateScore == 0) {
			this.chosenCandidateScore = score(candidate);
		}
		
		float secondCandidateScore = score(candidate);
		if(secondCandidateScore > chosenCandidateScore) {
			chosenCandidate = candidate;
		}
		else if(secondCandidateScore == chosenCandidateScore) {
			this.chosenCandidate = chooseRandomly(candidate, this.chosenCandidate);
		}
	}
	
	protected float score(Candidate candidate) {
		float score = 0;
		int[] attributes = candidate.getAttributes();
		for(int i = 0; i < attributes.length; ++i) {
			score += attributes[i] * attributesWeights[i];
			score /= attributes.length;
		}
		return score;
	}
	
	
	@Override
	public int calculateInfluence(int[] action, Candidate candidate) {
		int ret = 0;
		int[] attributes = candidate.getAttributes();
		for(int i = 0; i < action.length; ++i) {
			if(this.attributesWeights[i] + action[i] > 100) {
				ret += (100 - attributesWeights[i]) * attributes[i];
			}
			else if(this.attributesWeights[i] + action[i] < -100) {
				ret += (-100 - attributesWeights[i]) * attributes[i];
			}
			else {
				ret += action[i] * attributes[i];
			}
		}
		return ret;
	}
	
	@Override
	public void influenceOpinion(int[] action) {
		for(int i = 0; i < action.length; ++i) {
			if(this.attributesWeights[i] + action[i] > 100) {
				this.attributesWeights[i] = 100;
			}
			else if(this.attributesWeights[i] + action[i] < -100) {
				this.attributesWeights[i] = -100;
			}
			else {
				this.attributesWeights[i] += action[i];
			}
		}
	}
}
