package voter;

import main.Candidate;
import main.Party;

import java.util.List;
import java.util.Map;

/* Wyborca wszechstronny wybierający spośród jednej partii. */
public class VersatileVoterParty extends VersatileVoter {
	protected Party party;
	
	public VersatileVoterParty(String name, String surname, int numOfDistrict,
							   int[] attributeWeights, Party party) {
		super(name, surname, numOfDistrict, attributeWeights);
		this.party = party;
	}
	
	@Override
	public void vote(Map<Party, List<Candidate>> ticket) {
		super.iterateOverPartyList(ticket.get(party));
		this.chosenCandidate.addVote();
	}
}
