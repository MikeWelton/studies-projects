package voter;

import main.Candidate;
import main.Party;

import java.util.List;
import java.util.Map;
import java.util.Random;

/* Żelazny elektorat partyjny. */
public class FaithfulPartyVoter extends Voter {
	protected Party party;
	
	public FaithfulPartyVoter(String name, String surname, int numOfDistrict,
							  Party party) {
		super(name, surname, numOfDistrict);
		this.party = party;
	}
	
	@Override
	public void vote(Map<Party, List<Candidate>> ticket) {
		List<Candidate> listOfCandidates = ticket.get(party);
		Random random = new Random();
		int listSize = listOfCandidates.size();
		int num = random.nextInt(listSize);
		this.chosenCandidate = listOfCandidates.get(num);
		this.chosenCandidate.addVote();
	}
}
