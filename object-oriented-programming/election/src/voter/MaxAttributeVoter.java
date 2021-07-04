package voter;

import main.Candidate;
import main.Party;

import java.util.List;
import java.util.Map;

/* Maksymalizujący jednocechowy. */
public class MaxAttributeVoter extends Voter {
	protected int numOfAttribute;
	
	public MaxAttributeVoter(String name, String surname, int numOfDistrict,
							 int numOfAttribute) {
		super(name, surname, numOfDistrict);
		this.numOfAttribute = numOfAttribute;
	}
	
	@Override
	public void vote(Map<Party, List<Candidate>> ticket) {
		super.iterateOverTicket(ticket);
		this.chosenCandidate.addVote();
	}
	
	@Override
	protected void decideIfBetter(Candidate candidate) {
		if(candidate.getValueOfAttribute(numOfAttribute) >
				chosenCandidate.getValueOfAttribute(numOfAttribute)) {
			this.chosenCandidate = candidate;
		}
		else if(candidate.getValueOfAttribute(numOfAttribute) ==
				chosenCandidate.getValueOfAttribute(numOfAttribute)) {
			this.chosenCandidate = chooseRandomly(candidate, this.chosenCandidate);
		}
	}
}
